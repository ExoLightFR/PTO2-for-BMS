#pragma once

#ifndef _IVIBE_DATA_H
#define _IVIBE_DATA_H

// "FalconIntellivibeSharedMemoryArea"

class IntellivibeData
{
public:
    unsigned char AAMissileFired;   // how many AA missiles fired
    unsigned char AGMissileFired;   // how many maveric/rockets fired
    unsigned char BombDropped;      // how many bombs dropped
    unsigned char FlareDropped;     // how many flares dropped
    unsigned char ChaffDropped;     // how many chaff dropped
    unsigned char BulletsFired;     // how many bullets shot
    int           CollisionCounter; // collisions
    bool          IsFiringGun;      // gun is firing
    bool          IsEndFlight;      // ending the flight from 3d
    bool          IsEjecting;       // we've ejected
    bool          In3D;             // in 3D?
    bool          IsPaused;         // sim paused?
    bool          IsFrozen;         // sim frozen?
    bool          IsOverG;          // are G limits being exceeded?
    bool          IsOnGround;       // are we on the ground?
    bool          IsExitGame;       // did we exit Falcon?
    float         Gforce;           // what gforce we are feeling
    float         eyex, eyey, eyez; // where the eye is in relationship to the plane
    int           lastdamage;       // 1 to 8 depending on quadrant. Make this into an enum later
    float         damageforce;      // how big the hit was.
    unsigned int  whendamage;       // when the hit occured, game time in ms
};

#endif // _IVIBE_DATA_H
