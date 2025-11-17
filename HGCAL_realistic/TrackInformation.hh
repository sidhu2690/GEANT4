#ifndef TRACKINFORMATION_HH
#define TRACKINFORMATION_HH

#include "G4VUserTrackInformation.hh"
#include "G4VUserPrimaryParticleInformation.hh"
#include "globals.hh"

// For primary particles (before tracking starts)
class PrimaryParticleInformation : public G4VUserPrimaryParticleInformation {
public:
    PrimaryParticleInformation(G4int cumTr) : fCumTr(cumTr) {}
    virtual ~PrimaryParticleInformation() {}
    
    G4int GetCumTr() const { return fCumTr; }
    void SetCumTr(G4int cumTr) { fCumTr = cumTr; }
    
    virtual void Print() const override {
        G4cout << "PrimaryParticleInformation: cumTr = " << fCumTr << G4endl;
    }

private:
    G4int fCumTr;
};

// For tracks (during simulation)
class TrackInformation : public G4VUserTrackInformation {
public:
    TrackInformation(G4int cumTr) : fCumTr(cumTr) {}
    virtual ~TrackInformation() {}
    
    G4int GetCumTr() const { return fCumTr; }
    void SetCumTr(G4int cumTr) { fCumTr = cumTr; }
    
    virtual void Print() const override {
        G4cout << "TrackInformation: cumTr = " << fCumTr << G4endl;
    }

private:
    G4int fCumTr;
};

#endif
