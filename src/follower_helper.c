#include "global.h"
#include "data.h"
#include "event_scripts.h"
#include "follower_helper.h"

// Pool of "unconditional" follower messages, indexed by emotion.
const struct FollowerMessagePool gFollowerBasicMessages[FOLLOWER_EMOTION_LENGTH] = {
    [FOLLOWER_EMOTION_HAPPY] = {gFollowerHappyMessages, EventScript_FollowerGeneric, N_FOLLOWER_HAPPY_MESSAGES},
    [FOLLOWER_EMOTION_NEUTRAL] = {gFollowerNeutralMessages, EventScript_FollowerGeneric, N_FOLLOWER_NEUTRAL_MESSAGES},
    [FOLLOWER_EMOTION_SAD] = {gFollowerSadMessages, EventScript_FollowerGeneric, N_FOLLOWER_SAD_MESSAGES},
    [FOLLOWER_EMOTION_UPSET] = {gFollowerUpsetMessages, EventScript_FollowerGeneric, N_FOLLOWER_UPSET_MESSAGES},
    [FOLLOWER_EMOTION_ANGRY] = {gFollowerAngryMessages, EventScript_FollowerGeneric, N_FOLLOWER_ANGRY_MESSAGES},
    [FOLLOWER_EMOTION_PENSIVE] = {gFollowerPensiveMessages, EventScript_FollowerGeneric, N_FOLLOWER_PENSIVE_MESSAGES},
    [FOLLOWER_EMOTION_LOVE] = {gFollowerLoveMessages, EventScript_FollowerGeneric, N_FOLLOWER_LOVE_MESSAGES},
    [FOLLOWER_EMOTION_SURPRISE] = {gFollowerSurpriseMessages, EventScript_FollowerGeneric, N_FOLLOWER_SURPRISE_MESSAGES},
    [FOLLOWER_EMOTION_CURIOUS] = {gFollowerCuriousMessages, EventScript_FollowerGeneric, N_FOLLOWER_CURIOUS_MESSAGES},
    [FOLLOWER_EMOTION_MUSIC] = {gFollowerMusicMessages, EventScript_FollowerGeneric, N_FOLLOWER_MUSIC_MESSAGES},
    [FOLLOWER_EMOTION_POISONED] = {gFollowerPoisonedMessages, EventScript_FollowerGeneric, N_FOLLOWER_POISONED_MESSAGES},
};
