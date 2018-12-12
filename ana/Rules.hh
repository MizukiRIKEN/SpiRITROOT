#ifndef RULESSS_H
#define RULESSS_H
#include "HistToCutG.hh"
#include <vector>
#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <sstream>
#include <functional>


#include "TMath.h"
#include "TH1.h"
#include "TGraph.h"
#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TClonesArray.h"
#include "TCutG.h"
#include "TChain.h"

#include "STRecoTrack.hh"
#include "STEmbedTrack.hh"
#include "ST_ClusterNum_DB.hh"

typedef std::vector<std::vector<double>> DataSink;
typedef TTreeReaderValue<TClonesArray> ReaderValue;
class DrawMultipleComplex;

// apparently the compiler is not yet c++14 compitable
// that's why it's here
// apparently FairRoot has their own implementation of mylib::make_unique
// I have to name it differently..
namespace mylib
{
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args) {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
};

class Rule
{
public:
    friend class RecoTrackRule;
    friend class EmbedFilter;
    friend class UseVATracks;
    friend class DrawMultipleComplex;
    friend std::pair<Rule*, Rule*> RuleBlock(Rule* t_rule);

    Rule() : NextRule_(nullptr), PreviousRule_(nullptr), RejectRule_(nullptr) {};
    virtual ~Rule(){};

    virtual void SetReader(TTreeReader& t_reader); // unless you know what you are doing, dont override SetReader
    virtual void SetMyReader(TTreeReader& t_reader){}; // only override SetMyReader such that all inheriented class will set parent's reader
    virtual void Fill(std::vector<DataSink>& t_hist, int t_entry);
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) = 0;

    virtual Rule* AddRule(Rule* t_rule);
    void InsertRule(Rule* t_rule);
    virtual Rule* AddRejectRule(Rule* t_rule);

    int CloneTo(std::vector<std::unique_ptr<Rule>>& t_rule)
    {
        auto cloned_rule = this->Clone();
        t_rule.push_back(std::move(cloned_rule));
        int idx = t_rule.size()-1;
        std::cout << "IDX size " << idx << std::endl;
        if(this->NextRule_)
        {
            int nextidx = this->NextRule_->CloneTo(t_rule);
            std::cout << "NextRule idx " << nextidx << std::endl;
            t_rule[idx]->AddRule(t_rule[nextidx].get());
        }

        if(this->RejectRule_)
        {
            int rejectidx = this->RejectRule_->CloneTo(t_rule);
            t_rule[idx]->AddRejectRule(t_rule[rejectidx].get());
        }
        return idx;
    }

    void AppendRule(Rule* t_rule);
    void AppendRejectRule(Rule* t_rule);
    void PopRule();
//protected:
    virtual std::unique_ptr<Rule> Clone() = 0;
    inline void FillData(std::vector<DataSink>& t_hist, int t_entry) 
    {if(NextRule_) NextRule_->Fill(t_hist, t_entry);};
    inline void RejectData(std::vector<DataSink>& t_hist, int t_entry) 
    {if(RejectRule_) RejectRule_->Fill(t_hist, t_entry);};

    Rule* NextRule_;
    Rule* RejectRule_;
    Rule* PreviousRule_;
};

std::pair<Rule*, Rule*> RuleBlock(Rule* t_rule);

class EmptyRule : public Rule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override {this->FillData(t_hist, t_entry);};
    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<EmptyRule>(*this); };
};

class RecoTrackNumFilter : public Rule
{
public: 
    RecoTrackNumFilter(const std::function<bool(int)>& t_compare = [](int t_tracks){return t_tracks < 2;}) : compare_(t_compare){};
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<RecoTrackNumFilter>(*this); };
protected:
    std::shared_ptr<ReaderValue> myTrackArray_;
    std::function<bool(int)> compare_;
};

class CheckPoint : public Rule
{
public:
    CheckPoint(int t_id = 0) : id(t_id) {};
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override ;
    DataSink GetData();
    const int id;
    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<CheckPoint>(*this); };
protected:
    DataSink temp_sink_;
};

class Last2Rules2XY : public Rule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<Last2Rules2XY>(*this); };
};

std::vector<CheckPoint> ListOfCP(int t_num);
    
class DrawHit : public Rule
{
public: 
    DrawHit(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<DrawHit>(*this); };

protected:
    const int x_, y_;
    std::shared_ptr<ReaderValue> myHitArray_;
};

// this draws output of the previous rules
// will only work if it is DrawHit (or DrawTrack in the future). To do list
// Can only work with x-z or y-z plane. To do list for other dimensions
class GetHitOutline : public Rule
{
public: 
    GetHitOutline(const std::string& t_outputname);
    virtual ~GetHitOutline();
    virtual Rule* AddRule(Rule* t_rule) override;
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<GetHitOutline>(std::string(file_.GetName())); };

private:
    const int pad_x = 108;              
    const int pad_y = 112;
    const double size_x = 8;
    const double size_y = 12;
    const int max_num_ = 10000; // maximum number of cuts to be stored
    TFile file_;
    TClonesArray cutg_array_;                        // to do list: by pass this number
};

class DrawHitEmbed : public Rule
{
public: 
    DrawHitEmbed(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<DrawHitEmbed>(*this); };

protected:
    const int x_, y_;
    std::shared_ptr<ReaderValue> myHitArray_;
};


class ValueCut : public Rule
{
public:
    ValueCut(double t_lower=0, double t_upper=0, bool t_yaxis=false) : lower_(t_lower), upper_(t_upper)
    { index_ = (t_yaxis)? 1 : 0;};
    inline void SetCut(double t_lower, double t_upper) { lower_ = t_lower; upper_ = t_upper; };
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<ValueCut>(*this); };

protected:
    double lower_, upper_;
    int index_;
};
    
class EmbedCut : public Rule
{
public:
    EmbedCut() : cutg_(0) {};
    EmbedCut(const std::string& t_file, const std::string& t_cutname = "CUTG");    
    inline void SetCut(TCutG* t_cutg) { cutg_ = t_cutg; };
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    TCutG *GetCut() { return cutg_; };

    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<EmbedCut>(std::string(file_.GetName()), std::string(cutg_->GetName())); };
protected:
    TFile file_;
    TCutG* cutg_;
};

class XYCut : public Rule
{
public:
    XYCut(double t_xlower, double t_xupper, double t_ylower, double t_yupper) :
      xlower_(t_xlower), xupper_(t_xupper), ylower_(t_ylower), yupper_(t_yupper) {};

    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<XYCut>(*this);};
protected:
    double xlower_, xupper_, ylower_, yupper_;
};

class EntryRecorder : public Rule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    void ToFile(const std::string& t_filename);
    inline void Clear() { list_.clear(); };
    std::vector<int> GetList() { return list_; };

    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<EntryRecorder>(*this); };   
protected:
    std::vector<int> list_;
};

class TrackZFilter : public Rule
{
public:
    void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    std::unique_ptr<Rule> Clone() override { return mylib::make_unique<TrackZFilter>(*this); };   
protected:
    std::shared_ptr<ReaderValue> myTrackArray_;
};

#endif

