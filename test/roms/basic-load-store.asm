.segment "CODE"

reset:
  LDA #$42
  STA $0200

  BRK

irq_handler:
  LDA #$01
  STA $00FF

done:
  JMP done

.segment "VECTORS"
.word 0           ; NMI
.word reset       ; RESET
.word irq_handler ; IRQ/BRK