#include "GeoCuts_Radius.h"

int main(int argc, char** argv){
  // Settings
  bool primaryOnly, endVerticesAlso;

  char* inFilename = argv[1];
  char* outFilename = argv[2];
  double dCut = atof(argv[3]);
  std::stringstream ss4(argv[4]);
  ss4 >> std::boolalpha >> primaryOnly;
  std::stringstream ss5(argv[5]);
  ss5 >> std::boolalpha >> endVerticesAlso;
  std::string anaType(argv[6]);

  std::cout << "Using following settings:" << std::endl;
  std::cout << "Input Filename: " << inFilename << std::endl;
  std::cout << "Output Filename: " << outFilename << std::endl;
  std::cout << "Distance cut: " << dCut << std::endl;
  std::cout << "PrimaryOnly: " << std::boolalpha << primaryOnly << std::endl;
  std::cout << "EndVerticesAlso: " << std::boolalpha << endVerticesAlso << std::endl;
  std::cout << "AnalysisType: " << anaType << std::endl;

  TString trackProducer("pandoraNu");
  TString pfpProducer("pandoraNu");

  // Initialize variables that will go in the tree
  Int_t event, nTracks, nShowers, nPairs, nTrackVertices, nShowerVertices, nPotVertices, nCleanVertices, pandora_nPrimaryVertices, pandora_nCleanVertices;
  std::vector<float> pairDistance, potPairDistance;
  std::vector<int> pandora_primaryVertexPDG, pandora_nDaughters, pandora_nTracks, pandora_nShowers, pandora_nNearTracks, pandora_nNearShowers;

  // Generate root file and tree
  TFile tFile(outFilename, "RECREATE");
  TTree *tTree = new TTree("Data","Data");
  TTree *metaTree = new TTree("MetaData","MetaData");
  metaTree->Branch("distanceCut",&dCut,"dCut/D");
  metaTree->Branch("primaryOnly",&primaryOnly,"primaryOnly/O");
  metaTree->Branch("endVerticesAlso",&endVerticesAlso,"endVerticesAlso/O");
  metaTree->Branch("anaType",&anaType);
  tTree->Branch("event",&event,"event/I");
  tTree->Branch("nTracks",&nTracks,"nTracks/I");
  tTree->Branch("nShowers",&nShowers,"nShowers/I");
  tTree->Branch("pairDistance",&pairDistance);
  tTree->Branch("nPairs",&nPairs,"nPairs/I");
  tTree->Branch("nTrackVertices",&nTrackVertices,"nTrackVertices/I");
  tTree->Branch("nShowerVertices",&nShowerVertices,"nShowerVertices/I");
  tTree->Branch("potPairDistance",&potPairDistance);
  tTree->Branch("nPotVertices",&nPotVertices,"nPotVertices/I");
  tTree->Branch("nCleanVertices",&nCleanVertices,"nCleanVertices/I");
  tTree->Branch("pandora_nPrimaryVertices",&pandora_nPrimaryVertices,"pandora_nPrimaryVertices/I");
  tTree->Branch("pandora_primaryVertexPDG",&pandora_primaryVertexPDG);
  tTree->Branch("pandora_nDaughters",&pandora_nDaughters);
  tTree->Branch("pandora_nTracks",&pandora_nTracks);
  tTree->Branch("pandora_nShowers",&pandora_nShowers);
  tTree->Branch("pandora_nNearTracks",&pandora_nNearTracks);
  tTree->Branch("pandora_nNearShowers",&pandora_nNearShowers);
  tTree->Branch("pandora_nCleanVertices",&pandora_nCleanVertices,"pandora_nCleanVertices/I");

  // Set producer labels
  art::InputTag trackTag {trackProducer};
  art::InputTag pfpTag {pfpProducer};

  // Read file paths from .txt file
  std::vector<std::string> filenames;
  std::string file_name;
  std::ifstream input_file(inFilename);
  while (getline(input_file,file_name)) filenames.push_back(file_name);

  // Begin event loop
  for (gallery::Event ev(filenames); !ev.atEnd(); ev.next()){

    //----- STEP 1 -----//
    // Initialize. Get data to arrays //

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
    std::vector<recob::PFParticle> pandora_primaryPFP;
    std::vector<DecayVertex> trackVertices;
    std::vector<DecayVertex> showerVertices;

    // Cleaning vectors before starting loop
    pairDistance.clear();
    potPairDistance.clear();

    // Loop through each PFP
    int index = 0;
    for (auto const& pfp : (*pfpHandle)){
      // Generate temporary vectors to contain objects associated with pfp
      std::vector<recob::Track const*> tempTrackVector;
      std::vector<recob::Shower const*> tempShowerVector;

      // Fill temporary vectors with objects associated with the pfp
      pfp_to_track_assns.get(index,tempTrackVector);
      pfp_to_shower_assns.get(index,tempShowerVector);

      // Group all the primaries to analyze them later
      if (pfp.IsPrimary()){
        pandora_primaryPFP.push_back(pfp);
      }
      // Ignore (unphysical) reconstructed parents
      // (e.g. "ghost" neutrinos created by PandoraNu)
      else {
        // Find current pfp parent and check if parent is primary (so that pfp are only secondaries, and not tertiaries, etc.), then fill the correspondending vector with the type of particle found
        // tempTrackVector, tempShowerVector should always contain only ONE element; that exact track or shower (which is why .at(0)). No idea why it's a vector.
        if (primaryOnly){
          auto const& parent = (*pfpHandle).at(pfp.Parent());
          if (parent.IsPrimary()){
            if (tempTrackVector.size()!=0) tracks.push_back(tempTrackVector.at(0));
            if (tempShowerVector.size()!=0) showers.push_back(tempShowerVector.at(0));
          }
        }
        // Fill the track and shower vectors with the products associated with the pfp (in any case, regardless of their "genealogy")
        else {
          if (tempTrackVector.size()!=0) tracks.push_back(tempTrackVector.at(0));
          if (tempShowerVector.size()!=0) showers.push_back(tempShowerVector.at(0));
        }
        index++; // Index loops through pfps and MUST always keep on (no ifs)
      }
    } // End of pfp loop

    // Fill vertices vectors taking all the start and end points for tracks and start points for showers. The last two numbers (j,j) indicate only their idx in the xxxVertices vector (which is reduntant at this stage). However, later on, it will be used to identify the vertices uniquely, and the idxs from two vertices will be used to define the parent idx of the mean vertex created between them.
    // For tracks
    int j = 0;
    for(std::vector<int>::size_type i=0; i!=tracks.size(); i++){
      DecayVertex tempV1(tracks[i]->Vertex().X(),tracks[i]->Vertex().Y(),tracks[i]->Vertex().Z(),j,j);
      trackVertices.push_back(tempV1);
      j++;
      if (endVerticesAlso){
        DecayVertex tempV2(tracks[i]->End().X(),tracks[i]->End().Y(),tracks[i]->End().Z(),j,j);
        trackVertices.push_back(tempV2);
        j++;
      }
    }

    // And for showers
    for(std::vector<int>::size_type i=0; i!=showers.size(); i++){
      DecayVertex tempV(showers[i]->ShowerStart().X(),showers[i]->ShowerStart().Y(),showers[i]->ShowerStart().Z(),i,i);
      showerVertices.push_back(tempV);
    }


    //----- STEP 2a -----//
    //----- SDP ANALYSIS -----//
    // Analyze. Use arrays to get metadata //
    std::vector<DecayVertex> potVertices;
    std::vector<DecayVertex> cleanVertices;

    // Determine all potential mean vertices that satisfy the cut. Now the previously used (j,j) index are used to define the parent vertices that originated the mean vertex.
    // For track-track
    if (anaType == "tt"){
      for(std::vector<int>::size_type i=0; i!=trackVertices.size(); i++){
        for(std::vector<int>::size_type j=i+1; j!=trackVertices.size(); j++){
          DecayVertex v1 = trackVertices[i];
          DecayVertex v2 = trackVertices[j];
          float distance = Distance(v1,v2);
          pairDistance.push_back(distance);
          bool isInRadius = (distance<dCut);
          if (isInRadius) {
            DecayVertex v3 = MeanVertex(v1, v2);
            potVertices.push_back(v3);
            potPairDistance.push_back(distance);
          }
        }
      }
      // Make sure the potential mean vertices are clean (two decay vertices only in radius)
      // (start with assumption that isGoodVertex == true, then if you find extra particles in radius, turn it in bad vertex. Only good vertices are saved)
      for(std::vector<int>::size_type i=0; i!=potVertices.size(); i++){
        DecayVertex mv = potVertices[i];
        bool isGoodVertex = true;
        for(std::vector<int>::size_type j=0; j!=trackVertices.size(); j++){
          DecayVertex v1 = trackVertices[j];
          bool isParent1 = (mv.PID1() == v1.PID1());
          bool isParent2 = (mv.PID2() == v1.PID2());
          bool notParent = !(isParent1 || isParent2);
          bool isInRadius = (Distance(mv,v1)<dCut);
          if (isInRadius && notParent) isGoodVertex = false;
        }
        if (isGoodVertex) cleanVertices.push_back(mv);
      }
    }
    // And for track-shower
    else if (anaType == "ts"){
      for(std::vector<int>::size_type i=0; i!=trackVertices.size(); i++){
        for(std::vector<int>::size_type j=0; j!=showerVertices.size(); j++){
          DecayVertex v1 = trackVertices[i];
          DecayVertex v2 = showerVertices[j];
          float distance = Distance(v1,v2);
          pairDistance.push_back(distance);
          bool isInRadius = (distance<dCut);
          if (isInRadius) {
            DecayVertex v3 = MeanVertex(v1, v2);
            potVertices.push_back(v3);
            potPairDistance.push_back(distance);
          }
        }
      }
      // Make sure the potential mean vertices are clean (two decay vertices only in radius)
      // (start with assumption that isGoodVertex == true, then if you find extra particles in radius, turn it in bad vertex. Only good vertices are saved)
      for(std::vector<int>::size_type i=0; i!=potVertices.size(); i++){
        DecayVertex mv = potVertices[i];
        bool isGoodVertex = true;
        for(std::vector<int>::size_type j=0; j!=trackVertices.size(); j++){
          DecayVertex v1 = trackVertices[j];
          bool notParent = (mv.PID1() != v1.PID1());
          bool isInRadius = (Distance(mv,v1)<dCut);
          if (isInRadius && notParent) isGoodVertex = false;
        }
        for(std::vector<int>::size_type j=0; j!=showerVertices.size(); j++){
          DecayVertex v2 = showerVertices[j];
          bool notParent = (mv.PID2() != v2.PID2());
          bool isInRadius = (Distance(mv,v2)<dCut);
          if (isInRadius && notParent) isGoodVertex = false;
        }
        if (isGoodVertex) cleanVertices.push_back(mv);
      }
    }
    // Throw error if anaType wasn't right.
    else {
      throw std::invalid_argument("Invalid anaType. Must be 'tt' or 'ts'!");
    }

    // Determine other values
    event = event_n;
    nTracks = tracks.size();
    nShowers = showers.size();
    nPairs = pairDistance.size();
    nTrackVertices = trackVertices.size();
    nShowerVertices = showerVertices.size();
    nPotVertices = potVertices.size();
    nCleanVertices = cleanVertices.size();


    //----- STEP 2b -----//
    //----- PANDORA VERTEX ANALYSIS -----//
    pandora_primaryVertexPDG.clear();
    pandora_nDaughters.clear();
    pandora_nTracks.clear();
    pandora_nShowers.clear();
    pandora_nNearTracks.clear();
    pandora_nNearShowers.clear();
    pandora_nCleanVertices = 0;
    art::FindMany<recob::Vertex> pfp_to_vertex_assns(pfpHandle,ev,trackTag);
    // Repeat same analysis as before, this time, loop through each pfparticle which is a primary, find daughters, apply geo cut
    // Loop through each primary pfp
    for(std::vector<int>::size_type i=0; i!=pandora_primaryPFP.size(); i++){
      int temp_nTracks = 0, temp_nShowers = 0, temp_nNearTracks = 0, temp_nNearShowers = 0;
      // Get primary pfp vertex, (j,j) indices don't matter anymore, so just leave them by default to 0.
      recob::PFParticle pfp = pandora_primaryPFP[i];
      unsigned int nDaughters = pfp.NumDaughters();
      pandora_nDaughters.push_back(nDaughters);
      pandora_primaryVertexPDG.push_back(pfp.PdgCode());
      auto const& self_idx = pfp.Self();
      std::vector<recob::Vertex const*> tempVertexVector;
      pfp_to_vertex_assns.get(self_idx,tempVertexVector);
      auto const& pfpVertex = tempVertexVector.at(0);
      double tempCoords[3];
      pfpVertex->XYZ(tempCoords);
      DecayVertex v0(tempCoords[0],tempCoords[1],tempCoords[2],0,0);

      if (nDaughters!=0){
        for(size_t k=0; k!=nDaughters; k++){
          size_t daughter_idx = pfp.Daughters().at(k);
          std::vector<recob::Track const*> tempTrackVector;
          std::vector<recob::Shower const*> tempShowerVector;
          pfp_to_track_assns.get(daughter_idx,tempTrackVector);
          pfp_to_shower_assns.get(daughter_idx,tempShowerVector);
          if (tempTrackVector.size()==1){
            temp_nTracks += 1;
            auto const& track = tempTrackVector.at(0);
            DecayVertex v1(track->Vertex().X(),track->Vertex().Y(),track->Vertex().Z(),0,0);
            if (Distance(v0,v1)<dCut) temp_nNearTracks += 1;
            if (endVerticesAlso){
              DecayVertex v2(track->End().X(),track->End().Y(),track->End().Z(),0,0);
              if (Distance(v0,v2)<dCut) temp_nNearTracks += 1;
            }
          }
          else if (tempShowerVector.size()==1){
            temp_nShowers += 1;
            auto const& shower = tempShowerVector.at(0);
            DecayVertex v1(shower->ShowerStart().X(),shower->ShowerStart().Y(),shower->ShowerStart().Z(),0,0);
            if (Distance(v0,v1)<dCut) temp_nNearShowers += 1;
          }
          else{
            std::cout << "What the hell?!" << std::endl;
            std::cout << "Not primary has PDG: " << (*pfpHandle).at(daughter_idx).PdgCode() << " and nTracks: " << tempTrackVector.size() << std::endl;
          }
        }
      }
      if (anaType == "tt" && temp_nNearTracks==2) pandora_nCleanVertices += 1;
      if (anaType == "ts" && temp_nNearTracks==1 && temp_nNearShowers==1) pandora_nCleanVertices += 1;
      pandora_nTracks.push_back(temp_nTracks);
      pandora_nShowers.push_back(temp_nShowers);
      pandora_nNearTracks.push_back(temp_nNearTracks);
      pandora_nNearShowers.push_back(temp_nNearShowers);
    }// End of primary loop

    // Calculate final quantities
    pandora_nPrimaryVertices = pandora_primaryPFP.size();

    //----- STEP 3 -----//
    // Finalize. Save data to tree //
    // Fill the tree after each event loop
    tTree->Fill();
  } // End of event loop
  metaTree->Fill();

  // Save to file and close
  tFile.cd();
  tFile.Write();
  tFile.Close();
}
