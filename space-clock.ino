#include "space-clock.h"

void setup() {
  pinMode(inputs, INPUT_PULLUP);
  pinMode(pps, INPUT);

  input_interrupt = false;
  tick_interrupt = false;

  attachInterrupt(digitalPinToInterrupt(inputs), input, FALLING);
  attachInterrupt(digitalPinToInterrupt(pps), tick, RISING);

  cooldown = 0;

  Wire.begin();

  if (!gnss.begin()) {
//    while (1);
  }

  gnss.setI2COutput(COM_TYPE_UBX);
  gnss.setNavigationFrequency(4);

  if (!display.begin(0x70, 0x71)) {
//    while (1);
  }

  if (!button.begin()) {
//    while (1);
  }

  button.enableClickedInterrupt();
  button.clearEventBits();

  if (!rotary.begin()) {
//    while (1);
  }

  mode = Fix;
  zone = -7;
  dim = 0;

  rotary.setColor(48, 0, 255);
  display.setBrightness(dim);
}

void loop() {
  switch (mode) {
    case Fix:
      button.LEDoff();
      fix();
      break;
    case Idle:
      button.LEDoff();
      idle();
      break;
    case Zone:
      button.LEDon(LED_BRIGHTNESS);
      set_zone();
      break;
    case Dim:
      button.LEDon(LED_BRIGHTNESS);
      set_dim();
      break;
    case Elevation:
      button.LEDon(LED_BRIGHTNESS);
      elevation();
      break;
    case Latitude:
      button.LEDon(LED_BRIGHTNESS);
      latitude();
      break;
    case Longitude:
      button.LEDon(LED_BRIGHTNESS);
      longitude();
      break;
  }
}

void fix() {
  static uint16_t timer = 0;

  if (gnss.getFixType()) {
    mode = Idle;
    return;
  }

  if (timer > 900) {
    display.print(F("FIX FAIL"));
    while (1);
  } else {
    snprintf(
      buffer,
      BUFFER_LEN,
      "FIX +%03d",
      timer
    );

    display.print(buffer);
    timer++;
    delay(1000);
  }
}

void idle() {
  if (!gnss.getFixType()) {
    mode = Fix;
    return;
  }

  int8_t hour = gnss.getHour() + zone;
  if (hour < 0) hour = 24 + hour;

  if (tick_interrupt) {
    snprintf(
      buffer,
      BUFFER_LEN,
      "%02d:%02d  :%02d",
      hour,
      gnss.getMinute(),
      gnss.getSecond()
    );

    display.print(buffer);

    tick_interrupt = false;
  }

  if (input_interrupt) {
    if (button.hasBeenClicked()) {
      mode = Zone;
      button.clearEventBits();
      input_interrupt = false;
      return;
    }
  }
}

void set_zone() {
  if (handle_cooldown()) {
    mode = Idle;
    return;
  }

  if (input_interrupt) {
    if (rotary.isMoved()) {
      zone += rotary.getDiff();
      if (zone < -12) zone = -12;
      if (zone > 12) zone = 12;
      rotary.clearInterrupts();
      cooldown = 0;
      input_interrupt = false;
    }

    if (rotary.isClicked()) {
      mode = Idle;
      rotary.clearInterrupts();
      input_interrupt = false;
      return;
    }

    if (button.hasBeenClicked()) {
      mode = Dim;
      button.clearEventBits();
      input_interrupt = false;
      return;
    }
  }

  snprintf(
    buffer,
    BUFFER_LEN,
    "zone%+4d",
    zone
  );

  display.print(buffer);
}

void set_dim() {
  if (handle_cooldown()) {
    mode = Idle;
    return;
  }

  if (input_interrupt) {
    if (rotary.isMoved()) {
      dim += rotary.getDiff();
      if (dim > 15) dim = 15;
      rotary.clearInterrupts();
      cooldown = 0;
      input_interrupt = false;
    }

    if (rotary.isClicked()) {
      mode = Idle;
      rotary.clearInterrupts();
      input_interrupt = false;
      return;
    }

    if (button.hasBeenClicked()) {
      mode = Elevation;
      button.clearEventBits();
      input_interrupt = false;
      return;
    }
  }

  display.setBrightness(dim);

  snprintf(
    buffer,
    BUFFER_LEN,
    "DIM %+3d",
    dim
  );

  display.print(buffer);
}

void elevation() {
  if (input_interrupt) {
    if (button.hasBeenClicked()) {
      mode = Latitude;
      button.clearEventBits();
      input_interrupt = false;
      return;
    }
  }

  snprintf(
    buffer,
    BUFFER_LEN,
    "%lu",
    gnss.getAltitude()
  );

  display.print(buffer);
}

void latitude() {
  if (input_interrupt) {
    if (button.hasBeenClicked()) {
      mode = Longitude;
      button.clearEventBits();
      input_interrupt = false;
      return;
    }
  }

  snprintf(
    buffer,
    BUFFER_LEN,
    "%lu",
    gnss.getLatitude()
  );

  display.print(buffer);
}

void longitude() {
  if (input_interrupt) {
    if (button.hasBeenClicked()) {
      mode = Idle;
      button.clearEventBits();
      input_interrupt = false;
      return;
    }
  }

  snprintf(
    buffer,
    BUFFER_LEN,
    "%lu",
    gnss.getLongitude()
  );

  display.print(buffer);
}

int handle_cooldown() {
  if (!cooldown) {
    cooldown = millis() + 5000;
    return 0;
  } else {
    if (millis() > cooldown) {
      cooldown = 0;
      return 1;
    } else {
      return 0;
    }
  }
}

void input() {
  input_interrupt = true;
}

void tick() {
  tick_interrupt = true;
}
