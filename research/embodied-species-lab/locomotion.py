LOCOMOTION_VERSION = 1

REQUIRED_CLIP_NAMES = (
    "A_Idle",
    "A_TurnLeft",
    "A_Walk",
    "A_Run",
    "A_Stop",
    "A_Jump",
    "A_Fall",
    "A_Land",
)

ANIMATED_BONES = (
    "pelvis",
    "chest",
    "upper_arm_l",
    "upper_arm_r",
    "thigh_l",
    "thigh_r",
    "shin_l",
    "shin_r",
)


def _key(frame, **bones):
    pose = {name: [0.0, 0.0, 0.0] for name in ANIMATED_BONES}
    pose.update({name: list(value) for name, value in bones.items()})
    return {
        "frame": frame,
        "rootLocationMeters": [0.0, 0.0, 0.0],
        "rootEulerDegrees": [0.0, 0.0, 0.0],
        "boneEulerDegrees": {name: pose[name] for name in sorted(pose)},
    }


def _clip(name, end_frame, keys, looping=False, fps=30):
    return {
        "name": name,
        "fps": fps,
        "startFrame": 1,
        "endFrame": end_frame,
        "looping": looping,
        "keyframes": keys,
    }


def synthesize_locomotion_clips(fps=30):
    neutral = _key(1)
    return [
        _clip(
            "A_Idle",
            31,
            [neutral, _key(16, chest=(2, 0, 0)), _key(31)],
            True,
            fps,
        ),
        _clip(
            "A_TurnLeft",
            21,
            [
                neutral,
                _key(11, pelvis=(0, 0, 12), chest=(0, 0, -6)),
                _key(21),
            ],
            False,
            fps,
        ),
        _clip(
            "A_Walk",
            33,
            [
                _key(1, thigh_l=(25, 0, 0), thigh_r=(-25, 0, 0)),
                _key(9),
                _key(17, thigh_l=(-25, 0, 0), thigh_r=(25, 0, 0)),
                _key(25),
                _key(33, thigh_l=(25, 0, 0), thigh_r=(-25, 0, 0)),
            ],
            True,
            fps,
        ),
        _clip(
            "A_Run",
            25,
            [
                _key(
                    1,
                    thigh_l=(40, 0, 0),
                    thigh_r=(-40, 0, 0),
                    chest=(8, 0, 0),
                ),
                _key(7),
                _key(
                    13,
                    thigh_l=(-40, 0, 0),
                    thigh_r=(40, 0, 0),
                    chest=(8, 0, 0),
                ),
                _key(19),
                _key(
                    25,
                    thigh_l=(40, 0, 0),
                    thigh_r=(-40, 0, 0),
                    chest=(8, 0, 0),
                ),
            ],
            True,
            fps,
        ),
        _clip(
            "A_Stop",
            16,
            [
                _key(1, thigh_l=(20, 0, 0), thigh_r=(-20, 0, 0)),
                _key(8, thigh_l=(6, 0, 0), thigh_r=(-6, 0, 0)),
                _key(16),
            ],
            False,
            fps,
        ),
        _clip(
            "A_Jump",
            19,
            [
                _key(1),
                _key(
                    7,
                    thigh_l=(24, 0, 0),
                    thigh_r=(24, 0, 0),
                    shin_l=(-35, 0, 0),
                    shin_r=(-35, 0, 0),
                ),
                _key(13, upper_arm_l=(-25, 0, 0), upper_arm_r=(-25, 0, 0)),
                _key(19),
            ],
            False,
            fps,
        ),
        _clip(
            "A_Fall",
            19,
            [
                _key(1),
                _key(
                    10,
                    upper_arm_l=(0, 0, 35),
                    upper_arm_r=(0, 0, -35),
                    chest=(-8, 0, 0),
                ),
                _key(
                    19,
                    upper_arm_l=(0, 0, 45),
                    upper_arm_r=(0, 0, -45),
                    chest=(-12, 0, 0),
                ),
            ],
            True,
            fps,
        ),
        _clip(
            "A_Land",
            19,
            [
                _key(
                    1,
                    thigh_l=(28, 0, 0),
                    thigh_r=(28, 0, 0),
                    shin_l=(-40, 0, 0),
                    shin_r=(-40, 0, 0),
                ),
                _key(
                    10,
                    thigh_l=(10, 0, 0),
                    thigh_r=(10, 0, 0),
                    shin_l=(-15, 0, 0),
                    shin_r=(-15, 0, 0),
                ),
                _key(19),
            ],
            False,
            fps,
        ),
    ]
