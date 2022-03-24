/**
 * This header implements the Attestation  Manager class 
 */

#ifndef _ATTESTATION_MANAGER_H
#define _ATTESTATION_MANAGER_H

#include "fixed_types.h"
#include "dev_under_attestation.h"
#include "simulator.h"
#include "thread.h"
#include "thread_manager.h"
#include <vector>
#include <random>
#include <list>

using namespace std; 



class AttestationManager {
public:
    AttestationManager();
    virtual ~AttestationManager();

    void setAttestation(thread_id_t thread_id);
    UInt64 getChallengeHash(thread_id_t thread_id);
    UInt16 getChallengeId(thread_id_t thread_id);
    bool checkChallengeResult(thread_id_t thread_id, UInt128 challenge_result);
    
    
    //Tile * getTileFromId(tile_id_t tileId);
 /*    void printTileInfo();
    void registerThreadOnTile(thread_id_t threadId, core_id_t coreId);
    void unregisterThreadOnTile(thread_id_t threadId, core_id_t coreId);
    void setThreadSharedTime(thread_id_t threadId, UInt32 sharedTime);
    UInt32 getMaxSharedTimeOnTile(tile_id_t tileId);
    UInt32 getActiveThreadsOnTile(tile_id_t tileId);
    tile_id_t findTileFromThreadId(thread_id_t tileId);
    bool isSecure(tile_id_t tile_id) {return m_has_secure.at(tile_id);}
    void setSecure(core_id_t core_id);
    void unsetSecure(core_id_t core_id); */
    
private:
    std::mt19937 gentr;
    vector<DevUnderAttestation*> m_devices;
    DevUnderAttestation * getDevicebyThreadId(thread_id_t);
    UInt128 computeChallengeHash();
    UInt16 computeChallengeId();     
};

#endif
