.area _IVT


.area _HEADER
  nop
  jp 0x0150

  ; This is a bitmap of the Nintendo logo which is required for the cartridge
  ; to boot. The idea was to try to use their trademark to defend their
  ; licensing scheme. Apparently the courts ultimately decided they could not
  ; do this.
  .byte 0xce,0xed,0x66,0x66,0xcc,0x0d,0x00,0x0b,0x03,0x73,0x00,0x83,0x00,0x0c
  .byte 0x00,0x0d,0x00,0x08,0x11,0x1f,0x88,0x89,0x00,0x0e,0xdc,0xcc,0x6e,0xe6
  .byte 0xdd,0xdd,0xd9,0x99,0xbb,0xbb,0x67,0x63,0x6e,0x0e,0xec,0xcc,0xdd,0xdc
  .byte 0x99,0x9f,0xbb,0xb9,0x33,0x3e

  .ascii "OPEN SOURCE GB "
  ; .word 0xccdd,0xaabb
  .byte 0x00          ; no GBC compatibility
  .ascii "OS"         ; obviously not a licensee
  .byte 0x00          ; no SGB compatibility
  .byte 0x00          ; ROM-only cart type
  .byte 0x00          ; 32kB (non-banked) ROM
  .byte 0x00          ; no RAM
  .byte 0x01          ; international rom
  .byte 0x33          ; "defer to new licensee code"
  .byte 0x00          ; Version 0
  .byte 0xff          ; Header checksum (to be patched)
  .word 0xffff        ; Global checksum (to be patched)

.globl _init
.area _CODE
entry:
  di
  ld hl, #0xe000
  ld sp, hl
  ei
  jp _init

.area _DATA