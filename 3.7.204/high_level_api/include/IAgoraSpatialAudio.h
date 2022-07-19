//
//  AgoraRtcEngine SDK
//
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#ifndef AGORA_SPATIAL_AUDIO_H
#define AGORA_SPATIAL_AUDIO_H

#include <stdint.h>
#include "AgoraBase.h"
#include "AgoraMediaBase.h"
#include "AgoraRefPtr.h"
#include "IAgoraRtcEngineEx.h"

namespace agora {
namespace rtc {
/** connection state of Spacial Audio Service
 */
enum SAE_CONNECTION_STATE_TYPE {
  /* The SDK is connecting to the game server. */
  SAE_CONNECTION_STATE_CONNECTING = 0,
  /* The SDK is connected to the game server. */
  SAE_CONNECTION_STATE_CONNECTED,
  /* The SDK is disconnected from the game server. */
  SAE_CONNECTION_STATE_DISCONNECTED,
  /* The SDK is reconnecting to the game server. */
  SAE_CONNECTION_STATE_RECONNECTING,
  /* The SDK is reconnected to the game server. */
  SAE_CONNECTION_STATE_RECONNECTED
};

/** reason of connection state change of GME
 */
enum SAE_CONNECTION_CHANGED_REASON_TYPE {
  /* The connection state is changed. */
  SAE_CONNECTION_CHANGED_DEFAULT = 0,
  /* The SDK is connecting to the game server. */
  SAE_CONNECTION_CHANGED_CONNECTING,
  /* The SDK fails to create the game room. */
  SAE_CONNECTION_CHANGED_CREATE_ROOM_FAIL,
  /* The SDK is disconnected from the Agora RTM system. */
  SAE_CONNECTION_CHANGED_RTM_DISCONNECT,
  /* The SDK is kicked out of the Agora RTM system. */
  SAE_CONNECTION_CHANGED_RTM_ABORTED,
  /* The SDK recieved no message from server after long time */
  SAE_CONNECTION_CHANGED_LOST_SYNC
};

/** audio range mode type
 */
enum AUDIO_RANGE_MODE_TYPE {
  /* In world mode, you can hear players whose mode are also world mode in other teams */
  AUDIO_RANGE_MODE_WORLD = 0,
  /* In team mode, you can hear teammates only */
  AUDIO_RANGE_MODE_TEAM
};

// The information of remote voice position
struct RemoteVoicePositionInfo {
  // The coordnate of remote voice source, (x, y, z)
  float position[3];
  // The forward vector of remote voice, (x, y, z). When it's not set, the vector is forward to listner.
  float forward[3];

  RemoteVoicePositionInfo() = default;
};

/** IP areas.
 */
enum SAE_DEPLOY_REGION {
    /**
     * Mainland China.
     */
    SAE_DEPLOY_REGION_CN = 0x00000001,
    /**
     * North America.
     */
    SAE_DEPLOY_REGION_NA = 0x00000002,
    /**
     * Europe.
     */
    SAE_DEPLOY_REGION_EU = 0x00000004,
    /**
     * Asia, excluding Mainland China.
     */
    SAE_DEPLOY_REGION_AS = 0x00000008
};

/** The IGMEngineEventHandler class enables callbacks to your application.
 */
class ICloudSpatialAudioEventHandler {
 public:
  virtual ~ICloudSpatialAudioEventHandler() {}
  /**
   * Triggered when the current token will expire.
   *
   * This callback notifies the app to renew the token. The app should retrieve a new token from
   * the server and call renewToken to pass the new token to the SDK. Failure to do so results in
   * the SDK disconnecting from the server.
   */
  virtual void onTokenWillExpire() {}
  /**
   * Occurs when the connection state between the SDK and the server changes.
   *
   * @param state The current connection state. See #GME_CONNECTION_STATE_TYPE for all connection states.
   * @param reason The reason causing the change of the connection state. See #GME_CONNECTION_CHANGED_REASON_TYPE
   * for all reasons.
   */
  virtual void onConnectionStateChange(SAE_CONNECTION_STATE_TYPE state, SAE_CONNECTION_CHANGED_REASON_TYPE reason) {}

  virtual void onTeammateLeft(uid_t uid) {}
  virtual void onTeammateJoined(uid_t uid) {}
};

/** The definition of GMEngineContext
 */
struct CloudSpatialAudioConfig {
  /*The reference to \ref IRtcEngine, which is the base interface class of the Agora RTC SDK and provides
   * the real-time audio and video communication functionality.
   */
  agora::rtc::IRtcEngine* rtcEngine;
  /** The SDK uses the eventHandler interface class to send callbacks to the app.
   */
  ICloudSpatialAudioEventHandler* eventHandler;
  /** The App ID must be the same App ID used for initializing the IRtcEngine object.
   */
  const char* appId;
  /**
   * The region for connection. This advanced feature applies to scenarios that have regional restrictions.
   *
   * For the regions that Agora supports, see #SAE_DEPLOY_REGION. The area codes support bitwise operation.
   *
   * After specifying the region, the SDK connects to the Agora servers within that region.
   */
  unsigned int deployRegion;
  CloudSpatialAudioConfig() : rtcEngine(NULL), eventHandler(NULL), appId(NULL), deployRegion(SAE_DEPLOY_REGION_CN) {}
};

/** The definition of GMEngineContext
 */
struct LocalSpatialAudioConfig {
  /*The reference to \ref IRtcEngine, which is the base interface class of the Agora RTC SDK and provides
   * the real-time audio and video communication functionality.
   */
  agora::rtc::IRtcEngine* rtcEngine;

  LocalSpatialAudioConfig() : rtcEngine(NULL) {}
};

/** The IGameMediaEngine class provides the main methods that can be invoked by your application.
 */
class IBaseSpatialAudioEngine: public RefCountInterface {
 protected:
  virtual ~IBaseSpatialAudioEngine() {}

 public:
  /**
   * Releases all the resources occupied by spatial audio engine.
   */
  virtual void release() = 0;
  /**
   * This method sets the maximum number of streams that a player can receive in a
   * specified audio reception range.
   *
   * @note You can call this method either before or after calling enterRoom:
   * - Calling this method before enterRoom affects the maximum number of received streams
   *   the next time the player enters a room.
   * - Calling this method after entering a room changes the current maximum number of
   *   received streams of the player.
   *
   * @param maxCount The maximum number of streams that a player can receive within
   * a specified audio reception range. If the number of receivable streams exceeds
   * the set value, the SDK receives the set number of streams closest to the player.
   *
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int setMaxAudioRecvCount(int maxCount) = 0;
  /**
   * This method sets the audio reception range. The unit of the audio reception range
   * is the same as the unit of distance between game engines.
   *
   * @note You can call this method either before or after calling enterRoom.
   * During the game, you can call it multiple times to update the audio reception range.
   * To implement the range audio and spatial sound effects, you must use this method together
   * with `updateSelfPosition`.
   *
   * @param range The maximum audio reception range, in the unit of game engine distance.
   *
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int setAudioRecvRange(float range) = 0;

  /**
   * This method sets distance unit of game engine. The smaller the unit is, the sound fades slower
   * with distance.
   *
   * @note You can call this method either before or after calling enterRoom.
   * During the game, you can call it multiple times to update the distance unit.
   *
   * @param unit The number of meters that the game engine distance per unit is equal to. For example, setting unit as 2 means the game engine distance per unit equals 2 meters.
   *
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int setDistanceUnit(float unit) = 0;
  /**
   * Updates the sound source position of a player.
   * When calling it in ICloudSpatialAudioEngine, it triggers the SDK to update the sound source position of the player to the Agora server. The Agora server uses the players' world coordinates and audio reception range to determine whether they are within each other's specified audio reception range.
   * When calling it in ILocalSpatialAudioEngine, it triggers the SDK to calculate the relative position between the local and remote players and updates spatial audio parameters.
   *
   * when calling it in ICloudSpatialAudioEngine, you should notice:
   * @note
   * - Call the method after calling enterRoom.
   * - The call frequency is determined by the app. Agora recommends calling this method every
   *   50 to 2000 ms. Otherwise, the SDK may lose synchronization with the server.
   * - To use the range audio feature, you must call this method at least once after calling enterRoom.
   *
   * @param position The sound position of the player. The coordinate order is forward, right, and up.
   * @param axisForward The unit vector in the direction of the forward axis in the coordinate system.
   * @param axisRight The unit vector in the direction of the right axis in the coordinate system.
   * @param axisUp The unit vector in the direction of the up axis in the coordinate system.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int updateSelfPosition(float position[3], float axisForward[3], float axisRight[3], float axisUp[3]) = 0;
  /**
   * Updates the sound source position of a player. This method is used in scene with multi RtcConnection.
   *
   * @note
   * - This method is only effective in ILocalSpatialAudioEngine currently.
   *
   * @param position The sound position of the player. The coordinate order is forward, right, and up.
   * @param axisForward The unit vector in the direction of the forward axis in the coordinate system.
   * @param axisRight The unit vector in the direction of the right axis in the coordinate system.
   * @param axisUp The unit vector in the direction of the up axis in the coordinate system.
   * @param connection The RTC connection whose spatial audio effect you want to update.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int updateSelfPositionEx(float position[3], float axisForward[3], float axisRight[3], float axisUp[3], const RtcConnection& connection) = 0;

  /**
   * Updates the position of a media player in scene. This method has same behavior both in ICloudSpatialAudioEngine and ILocalSpatialAudioEngine.
   *
   * @note
   * - This method is suggested to be called once if you don't move media player in the virtual space.
   *
   * @param playerId The ID of the media player. You can get it by IMediaPlayer::getMediaPlayerId.
   * @param positionInfo The position information of media player in the virtual space. For details inforamtion, see the declaration of RemoteVoicePositionInfo.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int updatePlayerPositionInfo(int playerId, const RemoteVoicePositionInfo& positionInfo) = 0;

  /**
   * Set parameters for spatial audio engine. It's not deprecated for using.
   *
   * @param params The parameter string.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int setParameters(const char* params) = 0;

  /**
   * Mute or unmute local audio stream.
   *
   * @param mute When it's false, it will send local audio stream, otherwise it will not send local audio stream.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int muteLocalAudioStream(bool mute) = 0;
  /**
   * Mute all remote audio streams. It determines wether SDK receves remote audio streams or not.
   *
   * @param mute When it's false, SDK will receve remote audio streams, otherwise SDK will not receve remote audio streams.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int muteAllRemoteAudioStreams(bool mute) = 0;
};

class ICloudSpatialAudioEngine: public IBaseSpatialAudioEngine {
protected:
  virtual ~ICloudSpatialAudioEngine() {}

public:
  /**
   * Initializes the ICloudSpatialAudioEngine object and allocates the internal resources.
   *
   * @note Ensure that you call createAgoraGameMediaEngine and initialize before calling any other ICloudSpatialAudioEngine APIs.
   *
   * @param config The pointer to the CloudSpatialAudioConfig. See #CloudSpatialAudioConfig.
   *
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int initialize(const CloudSpatialAudioConfig& config) = 0;
  /**
   * Add event handler to deal with the callbacks of SDK.
   *
   * @param eventHandler The pointer to event handler. See #ICloudSpatialAudioEventHandler.
   *
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int addEventHandler(ICloudSpatialAudioEventHandler* eventHandler) = 0;
  /**
   * Remove event handler which deals with the callbacks of SDK.
   *
   * @param eventHandler The pointer to event handler. See #ICloudSpatialAudioEventHandler.
   *
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int removeEventHandler(ICloudSpatialAudioEventHandler* eventHandler) = 0;
  /**
   * This method enables the spatial sound effects.
   *
   * @note
   * - You can call this method either before or after calling enterRoom.
   * - Make sure `IRtcEngine::enableSoundPositionIndication` was called to use spacial audio feature.
   *
   * @param enable Whether to enable the spatial sound effects for the players within a specified audio range:
   * - true: Enable the spatial sound effects.
   * - false: Disable the spatial sound effects.

   * @param applyToTeam Whether to enable the spatial sound effects for the players in a team:
   * - true: Enable the spatial sound effects in a team.
   * - false: Disable the spatial sound effects in a team.
   *
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int enableSpatializer(bool enable, bool applyToTeam) = 0;
  /**
   * Sets the team ID of a player.
   *
   * The team ID setting affects the sound reachability between players in a game room as follows:
   * - The players with the same team ID are teammates. They can always hear each other, regardless of the audio mode and audio reception range.
   * - The players with different team IDs are opponents. Whether they can hear each other depends on the audio mode and
   *    audio reception range settings. For details, see Sound reachability between players.
   *
   * @note Call this method before calling `IGameMediaEngine::enterRoom`. If you call
   * this method after calling enterRoom, the SDK returns the error code -1.
   *
   * @param teamID The ID of a team. The value must be greater than 0. Setting the parameter to 0 means not teaming up with other players.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int setTeamId(int teamId) = 0;
  /**
   * Sets the audio mode of a player.
   *
   * @note You can call this method either before or after calling enterRoom:
   * - Calling this method before enterRoom affects the audio mode the next time the player enters the room.
   * - Calling this method after enterRoom changes the current audio mode of the player.
   *
   * @param rangeMode The audio mode:
   * - 0 : Everyone mode, in which the player can be heard by other players in the same room.
   * - 1: Team-only mode, in which only teammates can hear each other.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int setAudioRangeMode(AUDIO_RANGE_MODE_TYPE rangeMode) = 0;
  /**
   * Enters a game room.
   *
   * @note
   * - The range audio and spatial sound effects take effect after the method call of enterRoom.
   * - Call this method after calling `IRtcEngine::joinChannel`.
   *
   * @param token The token used for authentication. Ensure that the token used for entering
   * the room is the same token used in joinChannel.
   *
   * @param roomName The room name, which must be the same as the channelName used in joinChannel.
   * @param uid The user ID. The uid must be unique and not be set to 0.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int enterRoom(const char* token, const char* roomName, uid_t uid) = 0;
  /**
   * Renews the token when the current token expires.
   *
   * The app should retrieve a new token from the server and call this method to pass the new token
   * to the SDK. Otherwise, the SDK disconnects from the server when the token expires.
   * You can use this method to replace renewToken of the IRtcEngine class.
   *
   * @param token The new token. When generating the new token, ensure the following:
   * - Generate the new token in the same way as you generate the token for `IRtcEngine::joinChannel`.
   * - Add the permission to log into the Agora RTM system when generating Agora RTC token:
   *    builder.addPrivilege(AccessToken.Privileges.kRtmLogin, privilegeTs);
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int renewToken(const char* token) = 0;
  /**
   * Exits a game room.
   *
   * @return 0: Success.
   */
  virtual int exitRoom() = 0;

  virtual int getTeammates(const uid_t **uids, int *userCount) = 0;
};

class ILocalSpatialAudioEngine : public IBaseSpatialAudioEngine {
protected:
  virtual ~ILocalSpatialAudioEngine() {}
  
public:
  /**
   * Initializes the ILocalSpatialAudioEngine object and allocates the internal resources.
   *
   * @note Ensure that you call createAgoraGameMediaEngine and initialize before calling any other ILocalSpatialAudioEngine APIs.
   *
   * @param config The pointer to the LocalSpatialAudioConfig. See #LocalSpatialAudioConfig.
   *
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int initialize(const LocalSpatialAudioConfig& config) = 0;
  /**
   * Updates the position information of remote user. You should call it when remote user whose role is broadcaster move.
   *
   * @param uid The remote user ID. It should be the same as RTC channel remote user id.
   * @param posInfo The position information of remote user. See #RemoteVoicePositionInfo.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int updateRemotePosition(uid_t uid, const RemoteVoicePositionInfo &posInfo) = 0;
  /**
   * Updates the position of remote user. It's supposed to use with IRtcEngineEx::joinChannelEx.
   *
   * @param uid The remote user ID. It should be the same as RTC channel remote user id.
   * @param posInfo The position information of remote user. See #RemoteVoicePositionInfo.
   * @param connection The RTC connection whose spatial audio effect you want to update. See #RtcConnection.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int updateRemotePositionEx(uid_t uid, const RemoteVoicePositionInfo &posInfo, const RtcConnection& connection) = 0;
  /**
   * Remove the position information of remote user. You should call it when remote user called IRtcEngine::leaveChannel.
   *
   * @param uid The remote user ID. It should be the same as RTC channel remote user id.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int removeRemotePosition(uid_t uid) = 0;
  /**
   * Remove the position information of remote user. It's supposed to use with IRtcEngineEx::joinChannelEx.
   *
   * @param uid The remote user ID. It should be the same as RTC channel remote user id.
   * @param connection The RTC connection whose spatial audio effect you want to update. See #RtcConnection.
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int removeRemotePositionEx(uid_t uid, const RtcConnection& connection) = 0;
  /**
   * Clear the position informations of remote users.
   *
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int clearRemotePositions() = 0;
  /**
   * Clear the position informations of remote users. It's supposed to use with IRtcEngineEx::joinChannelEx.
   *
   * @return
   * - 0: Success.
   * - <0: Failure.
   */
  virtual int clearRemotePositionsEx(const RtcConnection& connection) = 0;
};

}  // namespace rtc
}  // namespace agora

#endif
