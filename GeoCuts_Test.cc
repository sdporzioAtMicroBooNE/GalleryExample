#include "GeoCuts_Test.h"

int main(int argv, char** argc){
  // Settings
  double distance_cut = 100.;
  TString trackProducer("pandoraNu");
  TString pfpProducer("pandoraNu");

  // Initialize variables that will go in the tree
  Int_t event, nTracks, nShowers, nPrimTracks, nPrimShowers, nTSPairs, nTTPairs, nPrimTSPairs, nPrimTTPairs;
  std::vector<float> TSdistance, TTdistance, primTSdistance, primTTdistance;

  // Generate root file and tree
  TFile tFile(argc[2], "RECREATE");
  TTree *tTree = new TTree("Tree","Tree");
  tTree->Branch("event",&event,"event/I");
  tTree->Branch("nTracks",&nTracks,"nTracks/I");
  tTree->Branch("nShowers",&nShowers,"nShowers/I");
  tTree->Branch("nTSPairs",&nTSPairs,"nTSPairs/I");
  tTree->Branch("nTTPairs",&nTTPairs,"nTTPairs/I");
  tTree->Branch("TSdistance",&TSdistance);
  tTree->Branch("TTdistance",&TTdistance);
  tTree->Branch("nPrimTracks",&nPrimTracks,"nPrimTracks/I");
  tTree->Branch("nPrimShowers",&nPrimShowers,"nPrimShowers/I");
  tTree->Branch("nPrimTSPairs",&nPrimTSPairs,"nPrimTSPairs/I");
  tTree->Branch("nPrimTTPairs",&nPrimTTPairs,"nPrimTTPairs/I");
  tTree->Branch("primTSdistance",&primTSdistance);
  tTree->Branch("primTTdistance",&primTTdistance);

  // Set producer labels
  art::InputTag trackTag {trackProducer};
  art::InputTag pfpTag {pfpProducer};

  // Read file paths from .txt file
  std::vector<std::string> filenames;
  std::string file_name;
  std::ifstream input_file(argc[1]);
  while (getline(input_file,file_name))
      filenames.push_back(file_name);

  // Begin event loop
  for (gallery::Event ev(filenames); !ev.atEnd(); ev.next()){
    // Cleaning vectors
    TSdistance.clear();
    TTdistance.clear();
    primTSdistance.clear();
    primTTdistance.clear();


    // Event information
    const auto& aux = ev.eventAuxiliary();
    const auto& event_n = aux.id().event();

    // Get PFParticles handle
    const auto& pfpHandle = ev.getValidHandle< std::vector<recob::PFParticle> >(pfpTag);

    // Find associations from PFP to tracks and showers
    art::FindMany<recob::Track> pfp_to_track_assns(pfpHandle,ev,trackTag);
    art::FindMany<recob::Shower> pfp_to_shower_assns(pfpHandle,ev,trackTag);

    // Prepare track and shower vectors
    std::vector<recob::Track const*> tracks;
    std::vector<recob::Shower const*> showers;
    std::vector<recob::Track const*> primTracks;
    std::vector<recob::Shower const*> primShowers;

    // Loop through each PFP
    int index = 0;
    for (auto const& pfp : (*pfpHandle)){
      // Extract everything associated to the pfp
      std::vector<recob::Track const*> tempTrack;
      std::vector<recob::Shower const*> tempShower;
      pfp_to_track_assns.get(index,tempTrack);
      pfp_to_shower_assns.get(index,tempShower);

      // Ignore (unphysical) reconstructed parents
      if (pfp.IsPrimary()!=1){

        // Fill the track and shower vectors with the products associated with the pfp (in any case)
        if (tempTrack.size()!=0) tracks.push_back(tempTrack.at(0));
        if (tempShower.size()!=0) showers.push_back(tempShower.at(0));

        // Find parent and check if parent is primary (so that pfp are only secondaries, and not tertiaries, etc.)
        auto const& parent = (*pfpHandle).at(pfp.Parent());
        if (parent.IsPrimary()){
          if (tempTrack.size()!=0) primTracks.push_back(tempTrack.at(0));
          if (tempShower.size()!=0) primShowers.push_back(tempShower.at(0));
        }
      }
    index++; // Index loops through pfps and MUST always keep on (no ifs)
    } // End of pfp loop

    // Determine distances between track-shower pairs
    for (auto const& track : (tracks)){
      for (auto const& shower : (showers)){
        std::vector<double> distance_value = GetTSVertexDistance(track,shower);
        for (auto const& d : distance_value){
          TSdistance.push_back(d);
        }
      }
    }
    // Determine distances between track-track pairs
    for(std::vector<int>::size_type i=0; i!=tracks.size(); i++){
      for(std::vector<int>::size_type j=i+1; j!=tracks.size(); j++){
        std::vector<double> distance_value = GetTTVertexDistance(tracks[i],tracks[j]);
        for (auto const& d : distance_value){
          TTdistance.push_back(d);
        }
      }
    }
    // Determine distances between track-shower pairs (only primaries)
    for (auto const& track : (primTracks)){
      for (auto const& shower : (primShowers)){
        std::vector<double> distance_value = GetTSVertexDistance(track,shower);
        for (auto const& d : distance_value){
          primTSdistance.push_back(d);
        }
      }
    }
    // Determine distances between track-track pairs (only primaries)
    for(std::vector<int>::size_type i=0; i!=primTracks.size(); i++){
      for(std::vector<int>::size_type j=i+1; j!=primTracks.size(); j++){
        std::vector<double> distance_value = GetTTVertexDistance(primTracks[i],primTracks[j]);
        for (auto const& d : distance_value){
          primTTdistance.push_back(d);
        }
      }
    }

    // Determine other values
    event = event_n;
    nTracks = tracks.size();
    nShowers = showers.size();
    nPrimTracks = primTracks.size();
    nPrimShowers = primShowers.size();
    nTTPairs = TTdistance.size();
    nTSPairs = TSdistance.size();
    nPrimTTPairs = primTTdistance.size();
    nPrimTSPairs = primTSdistance.size();

    // Fill the tree after each event loop
    tTree->Fill();
  }
  // End of event loop

  // Save to file and close
  tFile.cd();
  tFile.Write();
  tFile.Close();
}
