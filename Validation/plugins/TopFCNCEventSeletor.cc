#include "FWCore/Framework/interface/one/EDFilter.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CATTools/DataFormats/interface/Muon.h"
#include "CATTools/DataFormats/interface/Electron.h"
#include "CATTools/DataFormats/interface/Jet.h"
#include "CATTools/DataFormats/interface/MET.h"
//#include "CATTools/DataFormats/interface/SecVertex.h"
//#include "CATTools/DataFormats/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "CATTools/CommonTools/interface/TTbarModeDefs.h"
#include "CATTools/CommonTools/interface/ScaleFactorEvaluator.h"
#include "CATTools/CatAnalyzer/interface/TopTriggerSF.h"

#include "DataFormats/Candidate/interface/LeafCandidate.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CommonTools/Utils/interface/TFileDirectory.h"
#include "TH1D.h"
#include "TH2D.h"
#include "THnSparse.h"

#include <fstream>

#define nCutstep 11

using namespace std;

namespace cat {

struct ControlPlotsFCNC
{
  typedef TH1D* H1;
  typedef TH2D* H2;

  H1 hCutstep, hCutstepNoweight;

  H1 h_vertex_n[nCutstep];
  H1 h_met_pt[nCutstep], h_met_phi[nCutstep];
  H1 h_leptons_n[nCutstep];
  H1 h_lepton1_pt[nCutstep], h_lepton1_eta[nCutstep], h_lepton1_phi[nCutstep], h_lepton1_q[nCutstep];
  H1 h_jets_n[nCutstep], h_jets_pt[nCutstep], h_jets_eta[nCutstep], h_jets_ht[nCutstep];

  H1 h_jet_m[nCutstep][6];
  H1 h_jet_pt[nCutstep][6];
  H1 h_jet_eta[nCutstep][6];
  H1 h_jet_phi[nCutstep][6];
  H1 h_jet_btag[nCutstep][6];

  H1 h_bjets_n[nCutstep];
  H1 h_event_st[nCutstep];

  void book(TFileDirectory&& dir)
  {
    const double maxeta = 3;
    const double pi = 3.141592;

    hCutstep = dir.make<TH1D>("cutstep", "cutstep", nCutstep, 0, nCutstep);
    hCutstepNoweight = dir.make<TH1D>("cutstepNoweight", "cutstepNoweight", nCutstep, 0, nCutstep);

    const char* stepLabels[] = {
      "S1 all event", "S2 goodVertex", "S3 Event filter", "S4 trigger",
      "S5 lepton", "S6 other lepton veto", "S7 same lepton veto",
      "S8a nJet1", "S8b nJet2", "S8c nJet3", "S9  nBJet1"
    };
    const char* stepNames[nCutstep] = {"step1", "step2", "step3", "step4",
                                       "step5", "step6", "step7",
                                       "step8a", "step8b", "step8c", "step9"};

    for ( int i=0; i<nCutstep; ++i ) {
      hCutstep->GetXaxis()->SetBinLabel(i+1, stepLabels[i]);
      hCutstepNoweight->GetXaxis()->SetBinLabel(i+1, stepLabels[i]);
    }

    auto subdir = dir.mkdir(stepNames[0]);
    h_vertex_n[0] = subdir.make<TH1D>("vertex_n", "vertex_n;Number of primary vertices;Events;Vertex multipliity;Events", 100, 0, 100);

    for ( int i=1; i<=3; ++i ) {
      subdir = dir.mkdir(stepNames[i]);
      h_vertex_n[i] = subdir.make<TH1D>("vertex_n", "vertex_n;Number of primary vertices;Events", 100, 0, 100);
      h_met_pt[i] = subdir.make<TH1D>("met_pt", "met_pt;Missing transverse momentum (GeV);Events/1GeV", 1000, 0, 1000);
      h_met_phi[i] = subdir.make<TH1D>("met_phi", "met_phi;Missing transverse momentum #phi;Events", 100, -pi, pi);
      h_leptons_n[i] = subdir.make<TH1D>("leptons_n", "leptons_n;Lepton multiplicity;Events", 10, 0, 10);
      h_lepton1_pt[i]  = subdir.make<TH1D>("lepton1_pt", "lepton1_pt;1st leading lepton p_{T} (GeV);Events/1GeV", 1000, 0, 1000);
      h_lepton1_eta[i] = subdir.make<TH1D>("lepton1_eta", "lepton1_eta;1st leading lepton #eta;Events", 100, -maxeta, maxeta);
      h_lepton1_phi[i] = subdir.make<TH1D>("lepton1_phi", "lepton1_phi;1st leading lepton #phi;Events", 100, -pi, pi);
      h_lepton1_q[i]   = subdir.make<TH1D>("lepton1_q", "lepton1_q;1st leading lepton charge;Events", 3, -1.5, 1.5);
      h_jets_n[i] = subdir.make<TH1D>("jets_n", "jets_n;Jet multiplicity;Events", 10, 0, 10);
      h_jets_pt[i]  = subdir.make<TH1D>("jets_pt", "jets_pt;Jets p_{T} (GeV);Events/1GeV", 1000, 0, 1000);
      h_jets_eta[i] = subdir.make<TH1D>("jets_eta", "jets_eta;Jets #eta;Events", 100, -maxeta, maxeta);
      h_jets_ht[i] = subdir.make<TH1D>("jets_ht", "jets_ht;Jets #Sigma p_{T} (GeV);Events/1GeV", 1000, 0, 1000);
      h_bjets_n[i] = subdir.make<TH1D>("bjets_n", "bjets_n;b-jet multiplicity;Events", 10, 0, 10);
    }

    for ( int i=4; i<nCutstep; ++i ) {
      subdir = dir.mkdir(stepNames[i]);
      h_vertex_n[i] = subdir.make<TH1D>("vertex_n", "vertex_n;Number of primary vertices;Events", 100, 0, 100);
      h_met_pt[i] = subdir.make<TH1D>("met_pt", "met_pt;Missing transverse momentum (GeV);Events/1GeV", 1000, 0, 1000);
      h_met_phi[i] = subdir.make<TH1D>("met_phi", "met_phi;Missing transverse momentum #phi;Events", 100, -pi, pi);
      h_leptons_n[i] = subdir.make<TH1D>("leptons_n", "leptons_n;Lepton multiplicity;Events", 10, 0, 10);

      h_lepton1_pt[i]  = subdir.make<TH1D>("lepton1_pt", "lepton1_pt;1st leading lepton p_{T} (GeV);Events/1GeV", 1000, 0, 1000);
      h_lepton1_eta[i] = subdir.make<TH1D>("lepton1_eta", "lepton1_eta;1st leading lepton #eta;Events", 100, -maxeta, maxeta);
      h_lepton1_phi[i] = subdir.make<TH1D>("lepton1_phi", "lepton1_phi;1st leading lepton #phi;Events", 100, -pi, pi);
      h_lepton1_q[i]   = subdir.make<TH1D>("lepton1_q", "lepton1_q;1st leading lepton charge;Events", 3, -1.5, 1.5);

      h_jets_n[i] = subdir.make<TH1D>("jets_n", "jets_n;Jet multiplicity;Events", 10, 0, 10);
      h_jets_pt[i]  = subdir.make<TH1D>("jets_pt", "jets_pt;Jets p_{T} (GeV);Events/1GeV", 1000, 0, 1000);
      h_jets_eta[i] = subdir.make<TH1D>("jets_eta", "jets_eta;Jets #eta;Events", 100, -maxeta, maxeta);
      h_jets_ht[i] = subdir.make<TH1D>("jets_ht", "jets_ht;Jets #Sigma p_{T} (GeV);Events/1GeV", 1000, 0, 1000);

      for ( int j=0; j<6; ++j ) {
        const string prefix = Form("jet%d_", j+1);
        string titlePrefix = "";
        if ( j == 0 ) titlePrefix = "1st";
        else if ( j == 1 ) titlePrefix = "2nd";
        else if ( j == 2 ) titlePrefix = "3rd";
        else titlePrefix = Form("%dth", j+1);
        h_jet_m  [i][j] = subdir.make<TH1D>((prefix+"m").c_str(), (prefix+"m;"+titlePrefix+" leading jet mass (GeV);Events/1GeV").c_str(), 500, 0, 500);
        h_jet_pt [i][j] = subdir.make<TH1D>((prefix+"pt").c_str(), (prefix+"pt;"+titlePrefix+" leading jet p_{T} (GeV);Events/1GeV").c_str(), 1000, 0, 1000);
        h_jet_eta[i][j] = subdir.make<TH1D>((prefix+"eta").c_str(), (prefix+"eta;"+titlePrefix+" leading jet #eta;Events").c_str(), 100, -maxeta, maxeta);
        h_jet_phi[i][j] = subdir.make<TH1D>((prefix+"phi").c_str(), (prefix+"phi;"+titlePrefix+" leading jet #phi;Events").c_str(), 100, -pi, pi);
        h_jet_btag[i][j] = subdir.make<TH1D>((prefix+"btag").c_str(), (prefix+"btag;"+titlePrefix+" leading jet b discriminator output;Events").c_str(), 100, 0, 1);
      }

      h_bjets_n[i] = subdir.make<TH1D>("bjets_n", "bjets_n;b-jet multiplicity;Events", 10, 0, 10);

      h_event_st[i] = subdir.make<TH1D>("event_st", "event_st;#Sigma p_{T} *(GeV);Events/1GeV", 1000, 0, 1000);
    }
  };
};

class TopFCNCEventSelector : public edm::one::EDFilter<edm::one::SharedResources>
{
public:
  TopFCNCEventSelector(const edm::ParameterSet& pset);
  bool filter(edm::Event& event, const edm::EventSetup&) override;
  ~TopFCNCEventSelector();

private:
  typedef std::vector<float> vfloat;
  typedef std::vector<double> vdouble;
  int channel_;
  edm::EDGetTokenT<float> pileupWeightToken_, genWeightToken_;
  edm::EDGetTokenT<vfloat> genWeightsToken_;
  int genWeightIndex_;

  edm::EDGetTokenT<cat::MuonCollection> muonToken_;
  edm::EDGetTokenT<cat::ElectronCollection> electronToken_;
  edm::EDGetTokenT<cat::JetCollection> jetToken_;
  edm::EDGetTokenT<cat::METCollection> metToken_;
  edm::EDGetTokenT<reco::VertexCollection> vertexToken_;

  edm::EDGetTokenT<int> recoFilterToken_;
  edm::EDGetTokenT<int> trigElToken_, trigMuToken_;
  edm::EDGetTokenT<int> nVertexToken_;

  std::vector<edm::EDGetTokenT<float> > extWeightTokensF_;
  std::vector<edm::EDGetTokenT<double> > extWeightTokensD_;

private:
  double shiftedMuonScale(const cat::Muon& mu) { return 1+muonScale_*mu.shiftedEn()/mu.pt(); }
  double shiftedElectronScale(const cat::Electron& el) { return 1+electronScale_*el.shiftedEn()/el.pt(); }
  double shiftedJetScale(const reco::Candidate& cand)
  {
    const auto jet = dynamic_cast<const cat::Jet&>(cand);
    double scale = 1.0;
    if      ( jetScale_ == +1 ) scale *= jet.shiftedEnUp();
    else if ( jetScale_ == -1 ) scale *= jet.shiftedEnDown();

    if ( isMC_ and !isSkipJER_ ) scale *= jet.smearedRes(jetResol_);

    return scale;
  }

  bool isGoodMuon(const cat::Muon& mu)
  {
    if ( std::abs(mu.eta()) > 2.1 ) return false;
    if ( std::isnan(mu.pt()) or mu.pt() < 27 ) return false;

    if ( mu.relIso(0.4) > 0.15 ) return false;
    if ( !mu.isTightMuon() ) return false;
    return true;
  }
  bool isGoodElectron(const cat::Electron& el)
  {
    if ( std::abs(el.eta()) > 2.1 ) return false;
    if ( std::isnan(el.pt()) or el.pt() < 35 ) return false;

    if ( isMVAElectronSel_ and !el.isTrigMVAValid() ) return false;

    //if ( el.relIso(0.3) >= 0.11 ) return false;
    if ( !el.electronID(elIdName_) ) return false;
    //if ( !el.isPF() or !el.passConversionVeto() ) return false;
    const double scEta = std::abs(el.scEta());
    if ( isEcalCrackVeto_ and scEta > 1.4442 and scEta < 1.566 ) return false;
    //const double d0 = std::abs(el.dxy()), dz = std::abs(el.vz());
    //if      ( scEta <= 1.479 and (d0 > 0.05 or dz > 0.1) ) return false;
    //else if ( scEta >  1.479 and (d0 > 0.10 or dz > 0.2) ) return false;
    return true;
  }
  bool isVetoMuon(const cat::Muon& mu)
  {
    if ( std::abs(mu.eta()) > 2.4 ) return false;
    if ( std::isnan(mu.pt()) or mu.pt() < 10 ) return false;

    if ( !mu.isLooseMuon() ) return false;
    if ( mu.relIso(0.4) > 0.25 ) return false;
    return true;
  }
  bool isVetoElectron(const cat::Electron& el)
  {
    if ( std::abs(el.eta()) > 2.5 ) return false;
    if ( std::isnan(el.pt()) or el.pt() < 10 ) return false;
    if ( !el.electronID(elVetoIdName_) ) return false;
    //const double scEta = std::abs(el.scEta());
    //const double d0 = std::abs(el.dxy()), dz = std::abs(el.vz());
    //if      ( scEta <= 1.479 and (d0 > 0.05 or dz > 0.1) ) return false;
    //else if ( scEta >  1.479 and (d0 > 0.10 or dz > 0.2) ) return false;
    return true;
  }
  bool isBjet(const cat::Jet& jet)
  {
    using namespace cat;
    const double bTag = jet.bDiscriminator(bTagName_);
    if      ( bTagWP_ == BTagWP::CSVL ) return bTag > WP_BTAG_CSVv2L;
    else if ( bTagWP_ == BTagWP::CSVM ) return bTag > WP_BTAG_CSVv2M;
    else if ( bTagWP_ == BTagWP::CSVT ) return bTag > WP_BTAG_CSVv2T;
    return false;
  }

private:
  bool isUseGoodPV_; // Use good PV for the event selection or raw PV0

  // Energy scales
  int muonScale_, electronScale_, jetScale_, jetResol_;
  bool isSkipJER_; // Do not apply JER, needed to remove randomness during the Synchronization

  // Efficiency SF
  cat::ScaleFactorEvaluator muonSF_, electronSF_;
  double muonSFShift_, electronSFShift_;
  double trigSFShift_;

  bool isMC_;
  bool isIgnoreTrig_; // Accept event even if it does not pass HLT. Needed for synchronization
  const int applyFilterAt_;

  // ID variables
  bool isEcalCrackVeto_, isMVAElectronSel_;
  bool isSkipEleSmearing_; // Do not apply energy smearing, needed to remove randomness during the synchronization
  std::string bTagName_;
  std::string elIdName_, elVetoIdName_;
  enum class BTagWP { CSVL, CSVM, CSVT } bTagWP_;

private:
  ControlPlotsFCNC h_ch;

  std::ofstream eventListFile_;

};

} // namespace cat

using namespace cat;

TopFCNCEventSelector::TopFCNCEventSelector(const edm::ParameterSet& pset):
  isMC_(pset.getParameter<bool>("isMC")),
  applyFilterAt_(pset.getParameter<int>("applyFilterAt"))
{
  const string eventFileName = pset.getUntrackedParameter<std::string>("eventFile", "");
  if ( ! eventFileName.empty() ) eventListFile_.open(eventFileName);

  const auto channel = pset.getParameter<std::string>("channel");
  if ( channel == "electron" ) channel_ = 11;
  else if ( channel == "muon" ) channel_ = 13;
  else edm::LogError("TopFCNCEventSelector") << "channel must be \"electron\" or \"muon\"\n";

  const auto muonSet = pset.getParameter<edm::ParameterSet>("muon");
  muonToken_ = consumes<cat::MuonCollection>(muonSet.getParameter<edm::InputTag>("src"));
  muonScale_ = muonSet.getParameter<int>("scaleDirection");
  if ( isMC_ ) {
    const auto muonSFSet = muonSet.getParameter<edm::ParameterSet>("efficiencySF");
    // FIXME : for muons, eta bins are folded - always double check this with cfg
    muonSF_.set(muonSFSet.getParameter<vdouble>("pt_bins"),
                muonSFSet.getParameter<vdouble>("abseta_bins"),
                muonSFSet.getParameter<vdouble>("values"),
                muonSFSet.getParameter<vdouble>("errors"));
    muonSFShift_ = muonSet.getParameter<int>("efficiencySFDirection");
  }

  const auto electronSet = pset.getParameter<edm::ParameterSet>("electron");
  electronToken_ = consumes<cat::ElectronCollection>(electronSet.getParameter<edm::InputTag>("src"));
  elIdName_ = electronSet.getParameter<string>("idName");
  elVetoIdName_ = electronSet.getParameter<string>("vetoIdName");
  electronScale_ = electronSet.getParameter<int>("scaleDirection");
  if ( isMC_ ) {
    const auto electronSFSet = electronSet.getParameter<edm::ParameterSet>("efficiencySF");
    // FIXME : for electrons, eta bins are NOT folded - always double check this with cfg
    electronSF_.set(electronSFSet.getParameter<vdouble>("pt_bins"),
                    electronSFSet.getParameter<vdouble>("abseta_bins"),
                    electronSFSet.getParameter<vdouble>("values"),
                    electronSFSet.getParameter<vdouble>("errors"));
    electronSFShift_ = electronSet.getParameter<int>("efficiencySFDirection");
  }
  isEcalCrackVeto_ = isMVAElectronSel_ = false;
  isSkipEleSmearing_ = electronSet.getParameter<bool>("skipSmearing");
  if ( elIdName_.substr(0,3) == "mva" ) {
    isMVAElectronSel_ = true;
  }
  else {
    isEcalCrackVeto_ = electronSet.getParameter<bool>("applyEcalCrackVeto");
  }

  const auto jetSet = pset.getParameter<edm::ParameterSet>("jet");
  jetToken_ = consumes<cat::JetCollection>(jetSet.getParameter<edm::InputTag>("src"));
  jetScale_ = jetSet.getParameter<int>("scaleDirection");
  jetResol_ = jetSet.getParameter<int>("resolDirection");
  bTagName_ = jetSet.getParameter<string>("bTagName");
  const auto bTagWPStr = jetSet.getParameter<string>("bTagWP");
  if      ( bTagWPStr == "CSVL" ) bTagWP_ = BTagWP::CSVL;
  else if ( bTagWPStr == "CSVM" ) bTagWP_ = BTagWP::CSVM;
  else if ( bTagWPStr == "CSVT" ) bTagWP_ = BTagWP::CSVT;
  else edm::LogError("TopFCNCEventSelector") << "Wrong bTagWP parameter " << bTagWPStr;
  isSkipJER_ = jetSet.getParameter<bool>("skipJER");

  const auto metSet = pset.getParameter<edm::ParameterSet>("met");
  metToken_ = consumes<cat::METCollection>(metSet.getParameter<edm::InputTag>("src"));

  const auto vertexSet = pset.getParameter<edm::ParameterSet>("vertex");
  isUseGoodPV_ = vertexSet.getParameter<bool>("useGoodPV");
  nVertexToken_ = consumes<int>(vertexSet.getParameter<edm::InputTag>("nVertex"));
  vertexToken_ = consumes<reco::VertexCollection>(vertexSet.getParameter<edm::InputTag>("src"));
  pileupWeightToken_ = consumes<float>(vertexSet.getParameter<edm::InputTag>("pileupWeight"));

  const auto filterSet = pset.getParameter<edm::ParameterSet>("filters");
  recoFilterToken_ = consumes<int>(filterSet.getParameter<edm::InputTag>("filterRECO"));
  trigElToken_ = consumes<int>(filterSet.getParameter<edm::InputTag>("trigEL"));
  trigMuToken_ = consumes<int>(filterSet.getParameter<edm::InputTag>("trigMU"));
  isIgnoreTrig_ = filterSet.getParameter<bool>("ignoreTrig");
  trigSFShift_ = filterSet.getParameter<int>("efficiencySFDirection");

  if ( isMC_ ) {
    const auto genWeightSet = pset.getParameter<edm::ParameterSet>("genWeight");

    genWeightIndex_ = genWeightSet.getParameter<int>("index");
    if ( genWeightIndex_ < 0 ) genWeightToken_ = consumes<float>(genWeightSet.getParameter<edm::InputTag>("src"));
    else genWeightsToken_ = consumes<vfloat>(genWeightSet.getParameter<edm::InputTag>("src"));
  }

  // Other weights
  const auto extWeightLabels = pset.getParameter<std::vector<edm::InputTag> >("extWeights");
  for ( auto x : extWeightLabels ) {
    extWeightTokensF_.push_back(consumes<float>(x));
    extWeightTokensD_.push_back(consumes<double>(x));
  }

  // Fill histograms, etc
  usesResource("TFileService");
  edm::Service<TFileService> fs;
  const string channelStr = channel_ == 11 ? "el" : "mu";
  h_ch.book(fs->mkdir(channelStr));

  produces<int>("channel");
  produces<float>("weight");
  produces<float>("met");
  produces<float>("metphi");
  produces<std::vector<cat::Lepton> >("leptons");
  produces<std::vector<cat::Jet> >("jets");
}

bool TopFCNCEventSelector::filter(edm::Event& event, const edm::EventSetup&)
{
  if ( event.isRealData() ) isMC_ = false;

  // Get physics objects
  edm::Handle<cat::MuonCollection> muonHandle;
  event.getByToken(muonToken_, muonHandle);

  edm::Handle<cat::ElectronCollection> electronHandle;
  event.getByToken(electronToken_, electronHandle);

  edm::Handle<cat::JetCollection> jetHandle;
  event.getByToken(jetToken_, jetHandle);

  edm::Handle<cat::METCollection> metHandle;
  event.getByToken(metToken_, metHandle);
  const auto& metP4 = metHandle->at(0).p4();
  double metDpx = 0, metDpy = 0;

  edm::Handle<int> nVertexHandle;
  event.getByToken(nVertexToken_, nVertexHandle);
  const int nGoodVertex = *nVertexHandle;
  edm::Handle<reco::VertexCollection> vertexHandle;
  event.getByToken(vertexToken_, vertexHandle);
  // use the side-effect of catVertex producer: pv collection size == 1 only if the pv[0] is good vtx
  const bool isGoodPV0 = (nGoodVertex >= 1 and vertexHandle->size() == 1);

  std::auto_ptr<std::vector<cat::Lepton> > out_leptons(new std::vector<cat::Lepton>());
  std::auto_ptr<std::vector<cat::Jet> > out_jets(new std::vector<cat::Jet>());

  // Compute event weight - from generator, pileup, etc
  double weight = 1.0;
  if ( isMC_ ) {
    edm::Handle<float> fHandle;
    edm::Handle<vfloat> vfHandle;

    float genWeight = 1.0;
    if ( genWeightIndex_ < 0 ) {
      event.getByToken(genWeightToken_, fHandle);
      genWeight = *fHandle;
    }
    else {
      event.getByToken(genWeightsToken_, vfHandle);
      genWeight = vfHandle->at(genWeightIndex_);
    }

    event.getByToken(pileupWeightToken_, fHandle);
    const float puWeight = *fHandle;

    weight *= genWeight*puWeight;
    // NOTE: weight value to be multiplied by lepton SF, etc.
  }

  // Apply all other weights
  for ( auto t : extWeightTokensF_ ) {
    edm::Handle<float> h;
    if ( event.getByToken(t, h) ) weight *= *h;
  }
  for ( auto t : extWeightTokensD_ ) {
    edm::Handle<double> h;
    if ( event.getByToken(t, h) ) weight *= *h;
  }

  // Get event filters and triggers
  edm::Handle<int> trigHandle;
  event.getByToken(recoFilterToken_, trigHandle);
  const int isRECOFilterOK = *trigHandle;

  event.getByToken(trigElToken_, trigHandle);
  const int isTrigEl = *trigHandle;
  event.getByToken(trigMuToken_, trigHandle);
  const int isTrigMu = *trigHandle;

  double event_st = 0, event_ht = 0;

  // Select good leptons
  cat::MuonCollection selMuons, vetoMuons;
  for ( int i=0, n=muonHandle->size(); i<n; ++i ) {
    auto& p = muonHandle->at(i);
    const double scale = shiftedMuonScale(p);

    cat::Muon lep(p);
    lep.setP4(p.p4()*scale);
    if ( isGoodMuon(lep) ) selMuons.push_back(lep);
    else if ( isVetoMuon(lep) ) vetoMuons.push_back(lep);
    else continue;

    event_st += lep.pt();
    metDpx += lep.px()-p.px();
    metDpy += lep.py()-p.py();
  }
  cat::ElectronCollection selElectrons, vetoElectrons;
  for ( int i=0, n=electronHandle->size(); i<n; ++i ) {
    auto& p = electronHandle->at(i);
    const double scale = shiftedElectronScale(p)/(isSkipEleSmearing_ ? p.smearedScale() : 1);

    cat::Electron lep(p);
    lep.setP4(p.p4()*scale);
    if ( isGoodElectron(lep) ) selElectrons.push_back(lep);
    else if ( isVetoElectron(lep) ) vetoElectrons.push_back(lep);
    else continue;

    event_st += lep.pt();
    metDpx += lep.px()-p.px()/(isSkipEleSmearing_ ? p.smearedScale() : 1);
    metDpy += lep.py()-p.py()/(isSkipEleSmearing_ ? p.smearedScale() : 1);
  }
  std::sort(selElectrons.begin(), selElectrons.end(),
            [&](const cat::Electron& a, const cat::Electron& b){ return a.pt() > b.pt(); });
  std::sort(selMuons.begin(), selMuons.end(),
            [&](const cat::Muon& a, const cat::Muon& b){ return a.pt() > b.pt(); });

  const int leptons_n = selMuons.size() + selElectrons.size();
  const cat::Lepton* lepton1 = 0;
  double trigSF = 1, leptonSF = 1;
  if ( channel_ == 11 and !selElectrons.empty() ) {
    const auto& el = selElectrons.at(0);
    lepton1 = &el;
    leptonSF = electronSF_(el.pt(), std::abs(el.scEta()), electronSFShift_);
  }
  else if ( channel_ == 13 and !selMuons.empty() ) {
    const auto& mu = selMuons.at(0);
    lepton1 = &mu;
    leptonSF = muonSF_(mu.pt(), std::abs(mu.eta()), muonSFShift_);
  }
  int lepton1_id = 0;
  double lepton1_pt = -1, lepton1_eta = -999, lepton1_phi = -999;
  if ( lepton1 ) {
    lepton1_id = lepton1->pdgId();
    lepton1_pt = lepton1->pt();
    lepton1_eta = lepton1->eta();
    lepton1_phi = lepton1->phi();
    out_leptons->push_back(*lepton1);
  }

  // Select good jets
  int bjets_n = 0;
  for ( int i=0, n=jetHandle->size(); i<n; ++i ) {
    auto& p = jetHandle->at(i);
    if ( std::abs(p.eta()) > 2.4 ) continue;
    if ( !p.LooseId() ) continue;

    const double scale = shiftedJetScale(p);
    cat::Jet jet(p);
    jet.setP4(scale*p.p4());

    metDpx += jet.px()-p.px();
    metDpy += jet.py()-p.py();
    if ( jet.pt() < 30 ) continue;

    if ( lepton1 and deltaR(jet.p4(), lepton1->p4()) < 0.4 ) continue;

    event_ht += jet.pt();
    if ( isBjet(p) ) ++bjets_n;

    out_jets->push_back(jet);
  }
  event_st += event_ht;
  const int jets_n = out_jets->size();
  std::sort(out_jets->begin(), out_jets->end(),
            [&](const cat::Jet& a, const cat::Jet& b){return a.pt() > b.pt();});

  // Update & calculate met
  const double met_pt = hypot(metP4.px()-metDpx, metP4.py()-metDpy);
  const double met_phi = atan2(metP4.py()-metDpy, metP4.px()-metDpx);

  // Check cut steps
  std::vector<bool> cutsteps(nCutstep);
  cutsteps[0] = true; // always true
  cutsteps[1] = (isUseGoodPV_ ? (nGoodVertex >= 1) : isGoodPV0);
  cutsteps[2] = isRECOFilterOK;
  if ( channel_ == 11 ) {
    cutsteps[3] = (!isIgnoreTrig_ and isTrigEl != 0);
    cutsteps[4] = (selElectrons.size() == 1);
    cutsteps[5] = (selMuons.size()+vetoMuons.size() == 0);
    cutsteps[6] = vetoElectrons.empty();
  }
  else if ( channel_ == 13 ) {
    cutsteps[3] = (!isIgnoreTrig_ and isTrigMu != 0);
    cutsteps[4] = (selMuons.size() == 1);
    cutsteps[5] = (selElectrons.size()+vetoElectrons.size() == 0);
    cutsteps[6] = vetoMuons.empty();
  }
  cutsteps[ 7] = (jets_n >= 1);
  cutsteps[ 8] = (jets_n >= 2);
  cutsteps[ 9] = (jets_n >= 3);
  cutsteps[10] = (jets_n >= 4);

  // Run though the cut steps
  int cutstep = 0;
  double w = weight;
  for ( cutstep = 0; cutstep < nCutstep and cutsteps[cutstep]; ++cutstep ) {
    if ( cutstep == 3 and !isIgnoreTrig_ ) w *= trigSF;
    else if ( cutstep == 4 ) w *= leptonSF;
    h_ch.hCutstep->Fill(cutstep, w);
    h_ch.hCutstepNoweight->Fill(cutstep);

    h_ch.h_vertex_n[cutstep]->Fill(nGoodVertex, w);
    if ( cutstep >= 1 ) {
      h_ch.h_met_pt[cutstep]->Fill(met_pt, w);
      h_ch.h_met_phi[cutstep]->Fill(met_phi, w);
      h_ch.h_jets_ht[cutstep]->Fill(event_ht, w);

      h_ch.h_leptons_n[cutstep]->Fill(leptons_n, w);
      if ( lepton1 ) {
        h_ch.h_lepton1_pt[cutstep]->Fill(lepton1->pt(), w);
        h_ch.h_lepton1_eta[cutstep]->Fill(lepton1->eta(), w);
        h_ch.h_lepton1_phi[cutstep]->Fill(lepton1->phi(), w);
        h_ch.h_lepton1_q[cutstep]->Fill(lepton1->charge(), w);
      }

      h_ch.h_jets_n[cutstep]->Fill(jets_n, w);
      for ( int j=0, n=std::min(6, jets_n); j<n; ++j ) {
        h_ch.h_jets_pt[cutstep]->Fill(out_jets->at(j).pt(), w);
        h_ch.h_jets_eta[cutstep]->Fill(out_jets->at(j).eta(), w);
      }
      h_ch.h_bjets_n[cutstep]->Fill(bjets_n, w);
    }
    if ( cutstep >= 4 ) {
      h_ch.h_event_st[cutstep]->Fill(event_st, w);
      for ( int j=0, n=std::min(6, jets_n); j<n; ++j ) {
        h_ch.h_jet_m  [cutstep][j]->Fill(out_jets->at(j).mass(), w);
        h_ch.h_jet_pt [cutstep][j]->Fill(out_jets->at(j).pt(), w);
        h_ch.h_jet_eta[cutstep][j]->Fill(out_jets->at(j).eta(), w);
        h_ch.h_jet_phi[cutstep][j]->Fill(out_jets->at(j).phi(), w);
        h_ch.h_jet_btag[cutstep][j]->Fill(out_jets->at(j).bDiscriminator(bTagName_), w);
      }
    }
  }

  // Fill n-dim histogram
  double jet1_pt = -1, jet1_eta = -999, jet1_phi = -999;
  if ( jets_n >= 1 ) {
    const auto& jet = out_jets->at(0);
    jet1_pt  = jet.pt();
    jet1_eta = jet.eta();
    jet1_phi = jet.phi();
  }

  event.put(std::auto_ptr<int>(new int((int)channel_)), "channel");
  event.put(std::auto_ptr<float>(new float(weight)), "weight");
  event.put(std::auto_ptr<float>(new float(met_pt)), "met");
  event.put(std::auto_ptr<float>(new float(met_phi)), "metphi");
  event.put(out_leptons, "leptons");
  event.put(out_jets, "jets");

  // Apply filter at the given step.
  if ( cutstep >= applyFilterAt_ ) {
    if ( eventListFile_.is_open() ) {
      const int run = event.id().run();
      const int lum = event.id().luminosityBlock();
      const int evt = event.id().event();
      char buffer[101];
      snprintf(buffer, 100, "%6d %6d %10d", run, lum, evt);
      eventListFile_ << buffer;
      snprintf(buffer, 100, "  %+2d  %6.2f %+4.2f %+4.2f", lepton1_id, lepton1_pt, lepton1_eta, lepton1_phi);
      eventListFile_ << buffer;
      snprintf(buffer, 100, "    %6.1f  %+4.2f", met_pt, met_phi);
      eventListFile_ << buffer;
      snprintf(buffer, 100, "    %d %d", jets_n, bjets_n);
      eventListFile_ << buffer;
      snprintf(buffer, 100, "  %6.2f %+4.2f %+4.2f  \n", jet1_pt, jet1_eta, jet1_phi);
      eventListFile_ << buffer;
    }

    return true;
  }

  return false;
}

TopFCNCEventSelector::~TopFCNCEventSelector()
{
  if ( h_ch.hCutstepNoweight ) {
    cout << "---- cut flows without weight ----\n";
    if ( channel_ == 11 ) cout << "Electron channel:\n";
    else if ( channel_ == 13 ) cout << "Muon channel:\n";
    size_t fw = 8;
    for ( int i=1; i<=nCutstep; ++i ) {
      fw = std::max(fw, strlen(h_ch.hCutstepNoweight->GetXaxis()->GetBinLabel(i)));
    }
    fw += 2;
    for ( int i=1; i<=nCutstep; ++i ) {
      const string name(h_ch.hCutstepNoweight->GetXaxis()->GetBinLabel(i));
      cout << name;
      for ( size_t k=name.size(); k<fw; ++k ) cout << ' ';
      cout << h_ch.hCutstepNoweight->GetBinContent(i) << '\n';
    }
    cout << "-----------------------------------\n";
  }
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(TopFCNCEventSelector);

