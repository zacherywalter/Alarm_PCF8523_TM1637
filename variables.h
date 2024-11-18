// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F,           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
  };

const uint8_t DAY[7][4] = {
  //SUN
  { 0,
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,          //S
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,          //U
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F           //N
  },
  //MON
  { SEG_A | SEG_B | SEG_E | SEG_F,                  //M
    SEG_A | SEG_B | SEG_C | SEG_F,                  //M
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  //O
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F           //N
  },
  //tUES
  { SEG_D | SEG_E | SEG_F | SEG_G,                  //t
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,          //U
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,          //E
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G           //S
  },
  //WEd
  { SEG_C | SEG_D | SEG_E | SEG_F,                  //W
    SEG_B | SEG_C | SEG_D | SEG_E,                  //W
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,          //E
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G           //d
  },
  //tHUr
  { SEG_D | SEG_E | SEG_F | SEG_G,                  //t
    SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,          //H
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,          //U
    SEG_A | SEG_E | SEG_F                           //r
  },
  //Frid
  { SEG_A | SEG_E | SEG_F | SEG_G,                  //F
    SEG_A | SEG_E | SEG_F,                          //r
    SEG_E | SEG_F,                                  //i
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G           //d
  },
  //SAt
  { 0,
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,          //S
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  //A
    SEG_D | SEG_E | SEG_F | SEG_G                   //t
  }
};
