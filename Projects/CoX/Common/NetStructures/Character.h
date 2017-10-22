/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
//* Another small step toward a real Character server.

#include "CommonNetStructures.h"
#include "BitStream.h"
#include "Powers.h"

#include <QtCore/QString>
#include <cassert>
#include <string>
#include <vector>
enum WindowVisibility : uint32_t {
  wv_HideAlways = 0,
  wv_Always = 1,
  wv_OnMouseOver = 2,
  wv_Selected = 4,
};
#define MAX_CHARACTER_SLOTS 8
struct ClientOption
{
    enum eType
    {
        t_int = 1,
        t_string = 2,
        t_float = 3,
        t_sentence=4,
        t_quant_angle = 5,
        t_mat4 = 6,
        t_vec3 = 7,
        t_date = 9,
        t_unknown
    };
    struct Arg
    {
        Arg(int t,void *v) : type(eType(t)),tgt(v){}
        eType type;
        void *tgt;
    };
    std::string name;
    std::vector<Arg> m_args;
    //ClientOption(const char *v) : name(v) {}
};
class ClientOptions
{
    std::vector<ClientOption> m_opts;
    void init();
public:
    ClientOptions()
    {
        init();
        mouselook_scalefactor=0.6f;
    }
    bool    mouse_invert;
    float   mouselook_scalefactor;
    float   degrees_for_turns;
    int32_t control_debug;
    int32_t no_strafe;
    int32_t alwaysmobile;// 1- player is always mobile (can't be immobilized by powers)
    int32_t repredict;   //1 - client is out of sync with server, asking for new physics state info.
    int32_t neterrorcorrection;
    float   speed_scale;
    int32_t svr_lag,svr_lag_vary,svr_pl,svr_oo_packets,client_pos_id;
    int32_t atest0,atest1,atest2,atest3,atest4,atest5,atest6,atest7,atest8,atest9;
    int32_t predict,notimeout,selected_ent_server_index;
    bool m_ChatWindow_fading;
    bool m_NavWindow_fading;
    bool showTooltips=true;
    bool allowProfanity=true;
    bool chatBallons=true;
    WindowVisibility showArchetype=wv_OnMouseOver;
    WindowVisibility showSupergroup=wv_OnMouseOver;
    WindowVisibility showPlayerName=wv_OnMouseOver;
    WindowVisibility showPlayerBars=wv_OnMouseOver;
    WindowVisibility showVillainName=wv_OnMouseOver;
    WindowVisibility showVillainBars=wv_OnMouseOver;
    WindowVisibility showPlayerReticles=wv_OnMouseOver;
    WindowVisibility showVillainReticles=wv_OnMouseOver;
    WindowVisibility showAssistReticles=wv_OnMouseOver;
    uint8_t chatFontSize=0;
    ClientOption *get(int idx)
    {
        if(idx<0)
            return nullptr;
        assert((size_t(idx)<m_opts.size()) && "Unknown option requested!!");
        return &m_opts[idx];
    }

};
class Costume;

class Character : public NetStructure
{
friend  class CharacterDatabase;
typedef std::vector<PowerPool_Info> vPowerPool;
typedef std::vector<Costume *> vCostume;

        vPowerPool      m_powers;
        PowerTrayGroup  m_trays;
        QString         m_class_name;
        QString         m_origin_name;
        bool            m_full_options;
        ClientOptions   m_options;
        bool            m_first_person_view_toggle;
        uint8_t         m_player_collisions;
        float           m_unkn1,m_unkn2;
        uint32_t        m_unkn3,m_unkn4;
public:
                        Character();
//////////////////////////////////////////////////////////////////////////
// Getters and setters
        uint32_t        getLevel() const { return m_level; }
        void            setLevel(uint32_t val) { m_level = val; }
const   QString &       getName() const { return m_name; }
        void            setName(const QString &val);
const   QString &       getMapName() const { return m_mapName; }
        void            setMapName(const QString &val) { m_mapName = val; }
        uint8_t         getIndex() const { return m_index; }
        void            setIndex(uint8_t val) { m_index = val; }
        uint64_t        getAccountId() const { return m_owner_account_id; }
        void            setAccountId(uint64_t val) { m_owner_account_id = val; }
        uint64_t        getLastCostumeId() const { return m_last_costume_id; }
        void            setLastCostumeId(uint64_t val) { m_last_costume_id = val; }
//
//////////////////////////////////////////////////////////////////////////
        void            reset();
        bool            isEmpty();
        void            serializefrom(BitStream &buffer) override;
        void            serializeto(BitStream &buffer) const override;
        void            serialize_costumes(BitStream &buffer,bool all_costumes=true) const;
        void            serializetoCharsel(BitStream &bs);
        void            GetCharBuildInfo(BitStream &src); // serialize from char creation
        void            SendCharBuildInfo(BitStream &bs) const;
        void            recv_initial_costume(BitStream &src);
        Costume *       getCurrentCostume() const;
        void            DumpPowerPoolInfo( const PowerPool_Info &pool_info );
        void            DumpBuildInfo();
        void            face_bits(uint32_t){}
        void            dump();
        void            sendFullStats(BitStream &bs) const;
        //TODO: move these to some kind of Player info class
        void            sendTray(BitStream &bs);
        void            sendTrayMode(BitStream &bs) const;
        void            sendWindows(BitStream &bs) const;
        void            sendWindow(BitStream &bs) const;
        void            sendTeamBuffMode(BitStream &bs) const;
        void            sendDockMode(BitStream &bs) const;
        void            sendChatSettings(BitStream &bs) const;
        void            sendDescription(BitStream &bs) const;
        void            sendTitles(BitStream &bs) const;
        void            sendKeybinds(BitStream &bs) const;
        void            sendFriendList(BitStream &bs) const;
        void            sendOptions( BitStream &bs ) const;
        void            sendOptionsFull(BitStream &bs) const;
protected:
        PowerPool_Info  get_power_info(BitStream &src);
        uint64_t        m_owner_account_id;
        uint64_t        m_last_costume_id;
        uint8_t         m_index;
        uint32_t        m_level;
        QString         m_name;
        QString         m_mapName;
        bool            m_villain;
        vCostume        m_costumes;
        Costume *       m_sg_costume;
        uint32_t        m_current_costume_idx;
        bool            m_current_costume_set;
        uint32_t        m_num_costumes;
        bool            m_multiple_costumes; // has more then 1 costume
        bool            m_supergroup_costume; // player has a sg costume
        bool            m_using_sg_costume; // player uses sg costume currently
        enum CharBodyType
        {
            TYPE_MALE,
            TYPE_FEMALE,
            TYPE_UNUSED1,
            TYPE_UNUSED2,
            TYPE_HUGE,
            TYPE_NOARMS
        };
};
