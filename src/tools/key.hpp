#include "bits.hpp"

const unsigned KEY_LOW_PART{0};
const unsigned KEY_LOW_PART_MASK{umaxval(24)};
const unsigned KEY_HIGH_PART{24};
const unsigned KEY_HIGH_PART_MASK{umaxval(24)};

const unsigned KEY_ROTATION_OFFSETS[]{3, 7, 13, 19};
const unsigned KEY_ROTATION_OFFSETS_COUNT{sizeof(KEY_ROTATION_OFFSETS) /
                                          sizeof(KEY_ROTATION_OFFSETS[0])};

const unsigned DISKID_HIGH_ROTATION{30};
const unsigned DISKID_HIGH_ROTATION_MASK{umaxval(2)};
const unsigned DISKID_HIGH_PART{24};
const unsigned DISKID_HIGH_PART_MASK{umaxval(6)};
const unsigned DISKID_LOW_PART{0};
const unsigned DISKID_LOW_PART_MASK{umaxval(24)};

const unsigned SECRET_LOW_ROTATION{18};
const unsigned SECRET_LOW_ROTATION_MASK{umaxval(2)};
const unsigned SECRET_HIGH_PART{0};
const unsigned SECRET_HIGH_PART_MASK{umaxval(18)};
const unsigned SECRET_HIGH_PART_SHIFT{6};
