// c++ includes
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <exception>

// root includes
#include "TInterpreter.h"
#include "TROOT.h"
#include "TH1.h"
#include "TH2D.h"
#include "TH2I.h"
#include "TFile.h"
#include "TNtuple.h"
#include "TClonesArray.h"
#include "TCanvas.h"
#include "TGraph.h"

// art includes
#include "canvas/Utilities/InputTag.h"
#include "gallery/Event.h"
#include "gallery/ValidHandle.h"
#include "canvas/Persistency/Common/FindMany.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Persistency/Common/FindOne.h"

// larsoft object includes
#include "lardataobj/RecoBase/Track.h"
#include "lardataobj/RecoBase/Shower.h"
#include "lardataobj/RecoBase/PFParticle.h"
#include "lardataobj/RecoBase/TrackingTypes.h"
#include "lardataobj/Simulation/SimChannel.h"
#include "lardataobj/RawData/RawDigit.h"

std::vector<double> GetTSVertexDistance(const recob::Track* track, const recob::Shower* shower){
  std::vector<double> distance;
  TVector3 tVertex1 = track->Vertex();
  TVector3 tVertex2 = track->End();
  TVector3 sVertex = shower->ShowerStart();
  distance.push_back(sqrt(
  pow((tVertex1.X() - sVertex.X()),2.) +
  pow((tVertex1.Y() - sVertex.Y()),2.) +
  pow((tVertex1.Z() - sVertex.Z()),2.)));
  distance.push_back(sqrt(
  pow((tVertex2.X() - sVertex.X()),2.) +
  pow((tVertex2.Y() - sVertex.Y()),2.) +
  pow((tVertex2.Z() - sVertex.Z()),2.)));
  return distance;
}

std::vector<double> GetTTVertexDistance(const recob::Track* track1, const recob::Track* track2){
  std::vector<double> distance;
  TVector3 tVertex1 = track1->Vertex();
  TVector3 tVertex2 = track2->Vertex();
  TVector3 tVertex3 = track1->End();
  TVector3 tVertex4 = track2->End();

  distance.push_back(sqrt(
  pow((tVertex1.X() - tVertex2.X()),2.) +
  pow((tVertex1.Y() - tVertex2.Y()),2.) +
  pow((tVertex1.Z() - tVertex2.Z()),2.)));
  distance.push_back(sqrt(
  pow((tVertex1.X() - tVertex3.X()),2.) +
  pow((tVertex1.Y() - tVertex3.Y()),2.) +
  pow((tVertex1.Z() - tVertex3.Z()),2.)));
  distance.push_back(sqrt(
  pow((tVertex1.X() - tVertex4.X()),2.) +
  pow((tVertex1.Y() - tVertex4.Y()),2.) +
  pow((tVertex1.Z() - tVertex4.Z()),2.)));
  distance.push_back(sqrt(
  pow((tVertex2.X() - tVertex3.X()),2.) +
  pow((tVertex2.Y() - tVertex3.Y()),2.) +
  pow((tVertex2.Z() - tVertex3.Z()),2.)));
  distance.push_back(sqrt(
  pow((tVertex2.X() - tVertex4.X()),2.) +
  pow((tVertex2.Y() - tVertex4.Y()),2.) +
  pow((tVertex2.Z() - tVertex4.Z()),2.)));
  distance.push_back(sqrt(
  pow((tVertex3.X() - tVertex4.X()),2.) +
  pow((tVertex3.Y() - tVertex4.Y()),2.) +
  pow((tVertex3.Z() - tVertex4.Z()),2.)));
  return distance;
}

int factorial(int n){
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}
