# USB CDC

The PIC18F2550 exposes a USB CDC serial port alongside the existing UART.
Commands use the same binary frame on both transports: the first byte is the
number of following bytes, followed by the command and its parameters. Replies
are sent back on the transport that received the command.

The firmware configuration assumes a 20 MHz crystal and uses the PIC18F2550
PLL to provide the 48 MHz USB clock. Connect USB D- and D+ to RC4 and RC5, and
fit the capacitor required by the PIC18F2550 VUSB pin when using its internal
USB regulator. Software I2C uses RB0 for SDA and RB1 for SCL so it does not
reconfigure RC4, which is also USB D-. Both I2C lines require external pull-up
resistors. The USB stack runs in polling mode and assumes USB is attached; the
main loop must keep calling `USB_CDC_Task()` for enumeration and transfers.

The USB descriptors currently use the Microchip example VID/PID `04D8:000A`.
Replace these with identifiers assigned to the product before distributing
production devices.

The bundled USB device and CDC stack is from
[MicrochipTech/mla_usb](https://github.com/MicrochipTech/mla_usb), revision
`d28cc13c6b4a539ed2333348efbde7c6546fbfc6`, and is licensed under Apache 2.0.
See [mla_usb/LICENSE](./mla_usb/LICENSE) and
[mla_usb/README.md](./mla_usb/README.md).
