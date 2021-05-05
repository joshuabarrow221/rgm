#include <cstdlib>
#include <iostream>
#include <chrono>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TApplication.h>
#include <TROOT.h>
#include <TDatabasePDG.h>
#include <TLorentzVector.h>
#include <TCutG.h>
#include <TH1.h>
#include <TH2.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TBenchmark.h>
#include "clas12reader.h"
#include "HipoChain.h"

using namespace std;
using namespace clas12;

const double mN = 0.939;
const double mD = 1.8756;

double get_mmiss(TVector3 vbeam, TVector3 ve, TVector3 vp){
  
  double Ebeam = vbeam.Mag();
  double Ee = ve.Mag();
  double Ep = sqrt((mN * mN) + vp.Mag2());

  TVector3 vmiss = vbeam - ve - vp;
  double emiss = Ebeam + mD - Ee - Ep;
  double mmiss = sqrt((emiss * emiss) - vmiss.Mag2());

  return mmiss;
}

double get_phi_diff(double e_phi, double p_phi){

  if(e_phi>p_phi){
    if((e_phi-p_phi)<=180){
      return (e_phi-p_phi);
    }
    else{
      return 360 - (e_phi-p_phi);
    }
  }
  else{
    if((p_phi-e_phi)<=180){
      return (p_phi-e_phi);
    }
    else{
      return 360 - (p_phi-e_phi);
    }
  }
}

bool lowThetaCut(double theta, double chi2PID, double vtzDiff){
  
  if(theta > (50 * M_PI / 180)){
    return false;
  }
  if(fabs(chi2PID-0.459179)>(3*1.2085)){
    return false;
  }
  if(fabs(vtzDiff-0.484268)>(3*1.30286)){
    return false;
  }
  
  return true;
}
/*
void monitorPID(int argc, char** argv){

  if( argc < 3 ){
    cerr << "Incorrect number of arugments. Instead use:\n\t./code [outputRootFile] [inputFiles]... \n\n";
    return;
  }

  TFile * outFile = new TFile(argv[1],"RECREATE");
*/


void Usage()
{
  std::cerr << "Usage: ./monitorPID <MC =1,Data = 0> <path/to/input.hipo>  <path/to/output.root> \n\n\n";

}


int main(int argc, char ** argv){

  if(argc < 3)
    {
      Usage();
      return -1;
    }

  /////////////////////////////////////
  //  TString opt=gApplication->Argv(3);
  TString out = argv[2]; 
  TFile * outFile = new TFile(out,"RECREATE");
  vector<TH1*> hist_list_1;
  vector<TH2*> hist_list_2;

  TString mc= argv[1];
  bool isMC = false;
  if(mc == 1){isMC=true;}

  /////////////////////////////////////
  //Electron fiducials
  /////////////////////////////////////
  TH2D * h_Vcal_EoP = new TH2D("Vcal_EoP","Vcal vs. EoP ;Vcal;EoP",60,0,30,150,0.05,0.40);
  hist_list_2.push_back(h_Vcal_EoP);
  TH2D * h_Wcal_EoP = new TH2D("Wcal_EoP","Wcal vs. EoP ;Wcal;EoP",60,0,30,150,0.05,0.40);
  hist_list_2.push_back(h_Wcal_EoP);
  TH2D * h_phi_theta = new TH2D("phi_theta","phi vs. theta ;phi;theta",100,-180,180,100,5,40);
  hist_list_2.push_back(h_phi_theta);
  TH1D * h_sector = new TH1D("sector","sector;sector",6,1,7);
  hist_list_1.push_back(h_sector);

  /////////////////////////////////////
  //Electron Pid
  /////////////////////////////////////
  TH2D * h_P_EoP = new TH2D("P_EoP","P vs. EoP ;P;EoP",100,0,7,100,0.15,0.35);
  hist_list_2.push_back(h_P_EoP);
  TH1D * h_nphe = new TH1D("nphe","nphe;nphe",100,0,100);
  hist_list_1.push_back(h_nphe);
  
  /////////////////////////////////////
  //Electron Kinematics  
  /////////////////////////////////////
  TH1D * h_xB = new TH1D("xB","xB;xB",100,0,2);
  hist_list_1.push_back(h_xB);
  TH1D * h_QSq = new TH1D("QSq","QSq;QSq",100,0,3);
  hist_list_1.push_back(h_QSq);
  TH1D * h_WSq = new TH1D("WSq","WSq;WSq",100,0,7);
  hist_list_1.push_back(h_WSq);
  TH2D * h_xB_QSq = new TH2D("xB_QSq","xB vs. QSq ;xB;QSq",100,0,2,100,0,3);
  hist_list_2.push_back(h_xB_QSq);
  TH2D * h_xB_WSq = new TH2D("xB_WSq","xB vs. WSq ;xB;WSq",100,0,2,100,0,7);
  hist_list_2.push_back(h_xB_WSq);
  TH2D * h_QSq_WSq = new TH2D("QSq_WSq","QSq vs. WSq ;QSq;WSq",100,0,3,100,0,7);
  hist_list_2.push_back(h_QSq_WSq);

  /////////////////////////////////////
  //All Proton Angles
  /////////////////////////////////////
  TH1D * h_theta_L = new TH1D("theta_L","theta_L;theta_L",180,0,180);
  hist_list_1.push_back(h_theta_L);
  TH1D * h_theta_Lq = new TH1D("theta_Lq","theta_Lq;theta_Lq",180,0,180);
  hist_list_1.push_back(h_theta_Lq);

  /////////////////////////////////////
  //All Neutron Angles
  /////////////////////////////////////
  TH2D * h_ToM_eDep_nL = new TH2D("ToM_eDep_nL","ToM vs.eDep;ToM;eDep;events",100,0,15,100,0,50);
  hist_list_2.push_back(h_ToM_eDep_nL);
  TH1D * h_ToM_nL = new TH1D("ToM_nL","ToM;ToM;events",100,0,15);
  hist_list_1.push_back(h_ToM_nL);
  TH2D * h_mom_ToM_nL = new TH2D("mom_ToM_nL","mom vs. ToM;mom;ToM;events",100,0,2,100,0,15);
  hist_list_2.push_back(h_mom_ToM_nL);

  TH1D * h_mom_nL = new TH1D("mom_nL","mom;mom;events",100,0,2);
  hist_list_1.push_back(h_mom_nL);
  TH1D * h_theta_nL = new TH1D("theta_nL","theta_nL;theta_nL",180,0,180);
  hist_list_1.push_back(h_theta_nL);
  TH1D * h_theta_nLq = new TH1D("theta_nLq","theta_nLq;theta_nLq",180,0,180);
  hist_list_1.push_back(h_theta_nLq);
  TH1D * h_phi_e_nL = new TH1D("phi_e_nL","phi_e minus phi_nL;phi_e_nL",180,0,180);
  hist_list_1.push_back(h_phi_e_nL);

  /////////////////////////////////////
  //Lead Proton Checks
  /////////////////////////////////////
  TH1D * h_theta_L_FTOF = new TH1D("theta_L_FTOF","theta_L;theta_L",180,0,180);
  hist_list_1.push_back(h_theta_L_FTOF);
  TH1D * h_theta_Lq_FTOF = new TH1D("theta_Lq_FTOF","theta_Lq_FTOF;theta_Lq",180,0,180);
  hist_list_1.push_back(h_theta_Lq_FTOF);
  TH1D * h_phi_e_L = new TH1D("phi_e_L","phi_e minus phi_L;phi_e_L",180,0,180);
  hist_list_1.push_back(h_phi_e_L);
  TH2D * h_mmiss_phi_e_L = new TH2D("mmiss_phi_e_L","mmiss vs. phi_e minus phi_L;mmiss;phi_e_L",100,0.4,1.4,180,0,180);
  hist_list_2.push_back(h_mmiss_phi_e_L);
  TH2D * h_xB_mmiss = new TH2D("xB_mmiss","xB vs. mmiss;xB;mmiss",100,0,2,100,0.4,1.4);
  hist_list_2.push_back(h_xB_mmiss);
  TH2D * h_pmiss_mmiss = new TH2D("pmiss_mmiss","pmiss vs. mmiss;pmiss;mmiss",100,0,1.5,100,0.4,1.4);
  hist_list_2.push_back(h_pmiss_mmiss);
  TH2D * h_xB_theta_1q = new TH2D("xB_theta_1q","xB vs. theta_1q;xB;theta_1q",100,0,2,180,0,180);
  hist_list_2.push_back(h_xB_theta_1q);
  TH2D * h_Loq_theta_1q = new TH2D("Loq_theta_1q","Loq vs. theta_1q;Loq;theta_1q",100,0,1.5,180,0,180);
  hist_list_2.push_back(h_Loq_theta_1q);
  TH2D * h_pmiss_theta_miss = new TH2D("pmiss_theta_miss","pmiss vs theta_miss;pmiss;theta_miss",100,0,1.5,180,0,180);
  hist_list_2.push_back(h_pmiss_theta_miss);

  /////////////////////////////////////
  //Lead SRC Proton Checks
  /////////////////////////////////////
  TH1D * h_pmiss = new TH1D("pmiss","pmiss;pmiss",100,0,1.5);
  hist_list_1.push_back(h_pmiss);
  TH1D * h_mmiss = new TH1D("mmiss","mmiss;mmiss",100,0.4,1.4);
  hist_list_1.push_back(h_mmiss);
  TH2D * h_pmiss_theta_miss_SRC = new TH2D("pmiss_theta_miss_SRC","pmiss vs theta_miss SRC;pmiss;theta_1",100,0,1.5,180,0,180);
  hist_list_2.push_back(h_pmiss_theta_miss_SRC);
  TH2D * h_xB_Loq_SRC = new TH2D("xB_Loq","xB vs Loq SRC;xB;Loq",100,0,2,100,0,1.5);
  hist_list_2.push_back(h_xB_Loq_SRC);


  TH1D * h_pmiss_tight = new TH1D("pmiss_tight","pmiss;pmiss",100,0,1.5);
  hist_list_1.push_back(h_pmiss_tight);
  TH1D * h_mmiss_tight = new TH1D("mmiss_tight","mmiss;mmiss",100,0.4,1.4);
  hist_list_1.push_back(h_mmiss_tight);
  TH2D * h_pmiss_theta_miss_SRC_tight = new TH2D("pmiss_theta_miss_SRC_tight","pmiss vs theta_miss SRC;pmiss;theta_1",100,0,1.5,180,0,180);
  hist_list_2.push_back(h_pmiss_theta_miss_SRC_tight);
  
  /////////////////////////////////////
  //Recoil Nucleons
  /////////////////////////////////////
  TH1D * h_num_R = new TH1D("num_R","num Recoil;num_R",5,0,5);
  hist_list_1.push_back(h_num_R);
  TH1D * h_p_2 = new TH1D("p_2","p Recoil;p_2",100,0,1.5);
  hist_list_1.push_back(h_p_2);

  TH1D * h_num_R_tight = new TH1D("num_R_tight","num Recoil;num_R",5,0,5);
  hist_list_1.push_back(h_num_R_tight);
  TH1D * h_p_2_tight = new TH1D("p_2_tight","p Recoil;p_2",100,0,1.5);
  hist_list_1.push_back(h_p_2_tight);
  
  /////////////////////////////////////
  //Recoil SRC Nucleons
  /////////////////////////////////////
  TH1D * h_p_2_high = new TH1D("p_2_high","p Recoil;p_2",100,0,1.5);
  hist_list_1.push_back(h_p_2_high);
  TH1D * h_p_rel = new TH1D("p_rel","p Relative;p_rel",100,0,1.5);
  hist_list_1.push_back(h_p_rel);
  TH1D * h_p_cm = new TH1D("p_cm","p CM;p_cm",100,0,0.5);
  hist_list_1.push_back(h_p_cm);
  TH1D * h_p_t_cm = new TH1D("p_t_cm","p_t CM;p_t_cm",100,-0.5,0.5);
  hist_list_1.push_back(h_p_t_cm);
  TH1D * h_p_y_cm = new TH1D("p_y_cm","p_y CM;p_y_cm",100,-0.5,0.5);
  hist_list_1.push_back(h_p_y_cm);
  TH1D * h_p_x_cm = new TH1D("p_x_cm","p_x CM;p_x_cm",100,-0.5,0.5);
  hist_list_1.push_back(h_p_x_cm);
  TH1D * h_theta_rel = new TH1D("theta_rel","theta Relative;theta_rel",180,0,180);
  hist_list_1.push_back(h_theta_rel);
  TH2D * h_p_cm_theta_rel = new TH2D("p_cm_theta_rel","p CM vs. theta Relative;p_cm;theta_rel",100,0,0.5,180,0,180);
  hist_list_2.push_back(h_p_cm_theta_rel);

  TH1D * h_p_2_high_tight = new TH1D("p_2_high_tight","p Recoil;p_2",100,0,1.5);
  hist_list_1.push_back(h_p_2_high_tight);
  TH1D * h_p_rel_tight = new TH1D("p_rel_tight","p Relative;p_rel",100,0,1.5);
  hist_list_1.push_back(h_p_rel_tight);
  TH1D * h_p_cm_tight = new TH1D("p_cm_tight","p CM;p_cm",100,0,0.5);
  hist_list_1.push_back(h_p_cm_tight);
  TH1D * h_p_t_cm_tight = new TH1D("p_t_cm_tight","p_t CM;p_t_cm",100,-0.5,0.5);
  hist_list_1.push_back(h_p_t_cm_tight);
  TH1D * h_p_y_cm_tight = new TH1D("p_y_cm_tight","p_y CM;p_y_cm",100,-0.5,0.5);
  hist_list_1.push_back(h_p_y_cm_tight);
  TH1D * h_p_x_cm_tight = new TH1D("p_x_cm_tight","p_x CM;p_x_cm",100,-0.5,0.5);
  hist_list_1.push_back(h_p_x_cm_tight);
  TH1D * h_theta_rel_tight = new TH1D("theta_rel_tight","theta Relative;theta_rel",180,0,180);
  hist_list_1.push_back(h_theta_rel_tight);
  TH2D * h_p_cm_theta_rel_tight = new TH2D("p_cm_theta_rel_tight","p CM vs. theta Relative;p_cm;theta_rel",100,0,0.5,180,0,180);
  hist_list_2.push_back(h_p_cm_theta_rel_tight);


  for(int i=0; i<hist_list_1.size(); i++){
    hist_list_1[i]->Sumw2();
  }
  for(int i=0; i<hist_list_2.size(); i++){
    hist_list_2[i]->Sumw2();
  }
  
  int counter = 0;
  
  clas12root::HipoChain chain;
  TString inputFile = argv[3];

  chain.Add(inputFile);
  auto config_c12=chain.GetC12Reader();
  chain.SetReaderTags({0});
  auto& c12=chain.C12ref();

  double Ebeam = 4.247;
  if(isMC){Ebeam=6;}

  while(chain.Next()==true){
      //Display completed  
      counter++;
      if((counter%1000000) == 0){
	cerr << counter <<" completed \n";
      }    
      // get particles by type
      auto electrons=c12->getByID(11);
      auto protons=c12->getByID(2212);
      auto neutrons=c12->getByID(2112);
      double weight = 1;
      if(isMC){weight=c12->mcevent()->getWeight()/10000;}
      //cout<<"weight = "<<weight<<endl;
      

  /////////////////////////////////////
  //Electron fiducials
  /////////////////////////////////////
      if(electrons.size()!=1){continue;}
      TVector3 	p_b(0,0,Ebeam);
      TVector3 p_e;
      p_e.SetMagThetaPhi(electrons[0]->getP(),electrons[0]->getTheta(),electrons[0]->getPhi());
      double EoP_e =  (electrons[0]->cal(PCAL)->getEnergy() +  electrons[0]->cal(ECIN)->getEnergy() +  electrons[0]->cal(ECOUT)->getEnergy()) / p_e.Mag();
      
      h_Vcal_EoP->Fill(electrons[0]->cal(PCAL)->getLv(),EoP_e,weight);
      h_Wcal_EoP->Fill(electrons[0]->cal(PCAL)->getLw(),EoP_e,weight);
      h_phi_theta->Fill(p_e.Phi()*180/M_PI,p_e.Theta()*180/M_PI,weight);
      h_sector->Fill(electrons[0]->getSector(),weight);

      if(electrons[0]->cal(ECIN)->getLv() < 14){ continue; }
      if(electrons[0]->cal(ECIN)->getLw() < 14){ continue; }
	  
  /////////////////////////////////////
  //Electron Pid
  /////////////////////////////////////
      h_P_EoP->Fill(p_e.Mag(),EoP_e,weight);
      h_nphe->Fill(electrons[0]->che(HTCC)->getNphe(),weight);

      if(EoP_e < 0.18){ continue; }
      if(EoP_e > 0.28){ continue; }
      if(p_e.Mag() < 1){ continue; }
      if(p_e.Mag() > 6.6){ continue; }
      if(!isMC){
	if(electrons[0]->par()->getVz() < -5){continue;}
	if(electrons[0]->par()->getVz() > -1){continue;}
      }

  /////////////////////////////////////
  //Electron Kinematics  
  /////////////////////////////////////
      TVector3	p_q = p_b - p_e;
      double nu = Ebeam - p_e.Mag();
      double QSq = p_q.Mag2() - (nu*nu);
      double xB = QSq / (2 * mN * nu);
      double WSq = (mN*mN) - QSq + (2*nu*mN);
      
      h_xB->Fill(xB,weight);
      h_QSq->Fill(QSq,weight);
      h_WSq->Fill(WSq,weight);
      h_xB_QSq->Fill(xB,QSq,weight);
      h_xB_WSq->Fill(xB,WSq,weight);
      h_QSq_WSq->Fill(QSq,WSq,weight);

  /////////////////////////////////////
  //All Proton Angles
  /////////////////////////////////////
      int num_L = 0;
      int index_L = -1;      
      for(int j = 0; j < protons.size(); j++){
	TVector3 p_L;
	p_L.SetMagThetaPhi(protons[j]->getP(),protons[j]->getTheta(),protons[j]->getPhi());
	double theta_L = p_L.Theta() * 180 / M_PI;
	double theta_Lq = p_L.Angle(p_q) * 180 / M_PI;
	double vtz_L = protons[j]->par()->getVz();
	double Chi2Pid_L = protons[j]->par()->getChi2Pid();
	double vtz_diff = electrons[0]->par()->getVz()-vtz_L;
	if(isMC){vtz_diff=0.5;}
	h_theta_L->Fill(theta_L,weight);
	h_theta_Lq->Fill(theta_Lq,weight);

	if(((protons[j]->sci(FTOF1A)->getDetector()==12) || (protons[j]->sci(FTOF1B)->getDetector()==12)) || (protons[j]->sci(FTOF2)->getDetector()==12))
	  {
	    if(theta_Lq<25){
	      if(lowThetaCut(p_L.Theta(),Chi2Pid_L,vtz_diff)){
		num_L++;
		index_L=j;
	      }	  
	    }
	  }
      }      

  /////////////////////////////////////
  //All Neutron Angles
  /////////////////////////////////////
      for(int j = 0; j < neutrons.size(); j++){
	TVector3 p_nL;
	p_nL.SetMagThetaPhi(neutrons[j]->getP(),neutrons[j]->getTheta(),neutrons[j]->getPhi());
	double theta_nL = p_nL.Theta() * 180 / M_PI;
	double theta_nLq = p_nL.Angle(p_q) * 180 / M_PI;
	double phi_diff_n = get_phi_diff(p_e.Phi()*180/M_PI,p_nL.Phi()*180/M_PI);
	if(p_nL.Mag()<0.0001){continue;}
	
	int DET = 0;
	if(neutrons[j]->sci(FTOF1A)->getDetector()==12){DET=FTOF1A;}
	if(neutrons[j]->sci(FTOF1B)->getDetector()==12){DET=FTOF1B;}
	if(neutrons[j]->sci(FTOF2)->getDetector()==12){DET=FTOF2;}
	
	if(DET!=0){
	  double t_nL = neutrons[j]->sci(DET)->getTime() - c12->event()->getStartTime();;
	  double l_nL = neutrons[j]->sci(DET)->getPath();
	  double eDep_nL = neutrons[j]->sci(DET)->getEnergy();
	  double ToM = t_nL*100/l_nL;
	  h_ToM_eDep_nL->Fill(ToM,eDep_nL,weight);
	  if(eDep_nL>5){
	    h_ToM_nL->Fill(ToM,weight);
	    h_mom_ToM_nL->Fill(p_nL.Mag(),ToM,weight);
	    if(ToM>3.5){
	      h_mom_nL->Fill(p_nL.Mag(),weight);
	      h_theta_nL->Fill(theta_nL,weight);
	      if(theta_nL<35){
		h_theta_nLq->Fill(theta_nLq,weight);
		if(theta_nLq<25){
		  h_phi_e_nL->Fill(phi_diff_n,weight);
		}
	      }
	    }
	  }
	}
      }      
      
  /////////////////////////////////////
  //Lead Proton Checks
  /////////////////////////////////////
      bool isSRC_loose = false;
      bool isSRC_tight = false;
      if(num_L==1){
	TVector3 p_L;
	p_L.SetMagThetaPhi(protons[index_L]->getP(),protons[index_L]->getTheta(),protons[index_L]->getPhi());
	TVector3 p_1 = p_L - p_q;
	TVector3 p_miss = -p_1;
	double mmiss = get_mmiss(p_b,p_e,p_L);
	double phi_diff = get_phi_diff(p_e.Phi()*180/M_PI,p_L.Phi()*180/M_PI);
	double theta_L = p_L.Theta() * 180 / M_PI;
	double theta_miss = p_miss.Theta() * 180 / M_PI;
	double theta_Lq = p_L.Angle(p_q) * 180 / M_PI;
	double Loq = p_L.Mag() / p_q.Mag();
	double theta_1q = p_1.Angle(p_q) * 180 / M_PI;
	

	h_theta_L_FTOF->Fill(theta_L,weight);
	h_theta_Lq_FTOF->Fill(theta_Lq,weight);

	h_phi_e_L->Fill(phi_diff,weight);
	h_mmiss_phi_e_L->Fill(mmiss,phi_diff,weight);
	h_xB_mmiss->Fill(xB,mmiss,weight);
	h_pmiss_mmiss->Fill(p_miss.Mag(),mmiss,weight);
	h_xB_theta_1q->Fill(xB,theta_1q,weight);
	h_Loq_theta_1q->Fill(Loq,theta_1q,weight);
	h_pmiss_theta_miss->Fill(p_miss.Mag(),theta_miss,weight);
  /////////////////////////////////////
  //Lead SRC Proton Checks
  /////////////////////////////////////
	if(QSq>1.5){
	  if(p_miss.Mag()>0.25){
	    if((mmiss>0.84) && (mmiss<1.04)){
	      h_pmiss->Fill(p_miss.Mag(),weight);
	      h_mmiss->Fill(mmiss,weight);
	      h_pmiss_theta_miss_SRC->Fill(p_miss.Mag(),theta_miss,weight);
	      h_xB_Loq_SRC->Fill(xB,Loq,weight);
	      isSRC_loose=true;	      
	      if((Loq > 0.62) && (Loq < 0.96)){
		if(xB > 1.2){
		  h_pmiss_tight->Fill(p_miss.Mag(),weight);
		  h_mmiss_tight->Fill(mmiss,weight);
		  h_pmiss_theta_miss_SRC_tight->Fill(p_miss.Mag(),theta_miss,weight);	       
		  isSRC_tight=true;
		}
	      }
	    }
	  }
	}
      }

      
  /////////////////////////////////////
  //Recoil Nucleons
  /////////////////////////////////////
      if(isSRC_loose){
	h_num_R->Fill(protons.size()-1,1);
	if(isSRC_tight){
	  h_num_R_tight->Fill(protons.size()-1,1);
	}
      }		
      
      double mom_2 = 0;
      double mom_2_tight = 0;
      int index_2 = -1;
      int index_2_tight = -1;
      for(int j = 0; j < protons.size(); j++){
	if(j==index_L){continue;}
	TVector3 p_R;
	p_R.SetMagThetaPhi(protons[j]->getP(),protons[j]->getTheta(),protons[j]->getPhi());
	if(isSRC_loose){
	  h_p_2->Fill(p_R.Mag(),weight);
	  if(p_R.Mag()>mom_2){
	    index_2=j;
	    mom_2=p_R.Mag();
	  }
	  if(isSRC_tight){
	    h_p_2_tight->Fill(p_R.Mag(),weight);
	    if(p_R.Mag()>mom_2_tight){
	      index_2_tight=j;
	      mom_2_tight=p_R.Mag();
	    }
	  }
	}	
      }
   
  /////////////////////////////////////
  //Recoil SRC Nucleons
  /////////////////////////////////////
      if(index_2!=-1){
	//cout<< index_2 <<"\n";
	TVector3 p_L;
	p_L.SetMagThetaPhi(protons[index_L]->getP(),protons[index_L]->getTheta(),protons[index_L]->getPhi());
	TVector3 p_1 = p_L - p_q;
	TVector3 p_2;
	p_2.SetMagThetaPhi(protons[index_2]->getP(),protons[index_2]->getTheta(),protons[index_2]->getPhi());
	TVector3 p_rel = p_1-p_2;
	p_rel.SetMag(p_rel.Mag()/2);
	TVector3 p_cm = p_1+p_2;
	double theta_rel = p_1.Angle(p_2) * 180 / M_PI;
	
	//Create new reference frame
	TVector3 vt = p_2.Unit();
	TVector3 vy = p_2.Cross(p_q).Unit();
	TVector3 vx = vt.Cross(vy);

	h_p_2_high->Fill(p_2.Mag(),weight);
	if(p_2.Mag()>0.25){
	  h_p_rel->Fill(p_rel.Mag(),weight);
	  h_p_cm->Fill(p_cm.Mag(),weight);
	  h_p_t_cm->Fill(p_cm.Dot(vt),weight);
	  h_p_y_cm->Fill(p_cm.Dot(vy),weight);
	  h_p_x_cm->Fill(p_cm.Dot(vx),weight);
	  h_theta_rel->Fill(theta_rel,weight);
	  h_p_cm_theta_rel->Fill(p_cm.Mag(),theta_rel,weight);
	}
      }

      if(index_2_tight!=-1){
	TVector3 p_L;
	p_L.SetMagThetaPhi(protons[index_L]->getP(),protons[index_L]->getTheta(),protons[index_L]->getPhi());
	TVector3 p_1 = p_L - p_q;
	TVector3 p_2;
	p_2.SetMagThetaPhi(protons[index_2_tight]->getP(),protons[index_2_tight]->getTheta(),protons[index_2_tight]->getPhi());
	TVector3 p_rel = p_1-p_2;
	p_rel.SetMag(p_rel.Mag()/2);
	TVector3 p_cm = p_1+p_2;
	double theta_rel = p_1.Angle(p_2) * 180 / M_PI;	
	
	//Create new reference frame
	TVector3 vt = p_2.Unit();
	TVector3 vy = p_2.Cross(p_q).Unit();
	TVector3 vx = vt.Cross(vy);

	h_p_2_high_tight->Fill(p_2.Mag(),weight);
	if(p_2.Mag()>0.25){
	  h_p_rel_tight->Fill(p_rel.Mag(),weight);
	  h_p_cm_tight->Fill(p_cm.Mag(),weight);
	  h_p_t_cm_tight->Fill(p_cm.Dot(vt),weight);
	  h_p_y_cm_tight->Fill(p_cm.Dot(vy),weight);
	  h_p_x_cm_tight->Fill(p_cm.Dot(vx),weight);
	  h_theta_rel_tight->Fill(theta_rel,weight);
	  h_p_cm_theta_rel_tight->Fill(p_cm.Mag(),theta_rel,weight);
	}
      }
  }
    cout<<counter<<endl;

  outFile->cd();
  for(int i=0; i<hist_list_1.size(); i++){
    hist_list_1[i]->Write();
  }
  for(int i=0; i<hist_list_2.size(); i++){
    hist_list_2[i]->Write();
  }
  outFile->Close();
  cout<<"Finished making file: "<< out <<"\n";

}