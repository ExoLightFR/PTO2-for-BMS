<h1 align="center">PTO2 for BMS</h1>

<h3 align="center">A small utility to synchronize your Winwing PTO2 lights with Falcon BMS</h3>

<p align="center">
  <img src="preview.png" />
</p>


## Installation instructions

Download the `.exe` from the releases, drop it wherever you want, and run it. That's it!

**Windows defender might issue a warning.** That's normal! Click "More info" and "Run anyway". I can't pay a certificate to permanently avoid this warning. If you need proof that the software is safe, the source code is right there :)

## Configuration file

The executable creates a `PTO2_for_BMS.conf` file in your BMS `/User/Config` directory. You shouldn't edit it yourself, but you can share it with your friends to exchange configurations. If you delete it, the app will restore a default config file with the basics assigned.

## Notes

I has gotten a PTO2 for Christmas, and not being able to check the "3 greens" or the Master Caution on my panel was too frustrating. Inspired by [prestonflying's contributions](https://forum.dcs.world/topic/318859-custom-data-shown-on-ufc/) on the DCS forums with the Winwing UFC, I took it upon myself to make it work. Once it did, I thought I'd make a customisation UI so the tool could be used by my friends. I hope you'll find it useful!

The app uses `hidapi` to write HID output reports to the PTO2. I intercepted the messages that SimAppPro sends to the PTO2 to control the lights, and simply reproduced them in the software. The core implementation is very small, a few dozen lines of code. Everything else is just UI sugarcoating...
