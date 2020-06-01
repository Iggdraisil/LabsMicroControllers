byte crctable[] = {0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
                   0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
                   0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
                   0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
                   0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
                   0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
                   0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
                   0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
                   0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
                   0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
                   0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
                   0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
                   0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
                   0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
                   0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
                   0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
                  };

#define BAUD 9600
#define UBRRcalc 95  //(F_CPU/(BAUD*16L) - 1)

#define QUEUE_ELEMENTS 100
#define QUEUE_SIZE (QUEUE_ELEMENTS + 1)
byte Queue[QUEUE_SIZE];
int QueueIn, QueueOut;

void QueueInit(void)
{
    QueueIn = QueueOut = 0;
}

int QueuePut(byte number)
{
    if (QueueIn == ((QueueOut - 1 + QUEUE_SIZE) % QUEUE_SIZE))
    {
        return -1; /* Queue Full*/
    }

    Queue[QueueIn] = number;

    QueueIn = (QueueIn + 1) % QUEUE_SIZE;

    return 0; // No errors
}

int QueueGet()
{
    if (QueueIn == QueueOut)
    {
        return -1; /* Queue Empty - nothing to get*/
    }

    UDR0 = Queue[QueueOut];

    QueueOut = (QueueOut + 1) % QUEUE_SIZE;

    return 0; // No errors
}


const byte endMessage[] {0xF0, 0xF0, 0xF0};
const byte  errorOccured[] {0xFF, 0xFF, 0xFF};

const byte slave1 = 0x37;
const byte slave2 = 0x41;

const byte requestInfo1[3] = { slave1, 0x09, 0x57 };
const byte requestInfo2[3] = { slave2, 0x09, 0x57 };

bool packetAvailable = false;


byte packet_to_send[3];

bool clean_package = true;

byte packets[3][3];
byte dropped_packets[3] = { 0xC8, 0xC8, 0xC8 };
int byteCount = 0;
int packet_counter = 0;
int global_packet_counter = 0;

byte Compute_CRC8(const byte bytes[3])
{
  byte crc = 0;
  for (int i = 0; i < 3; i++)
  {
    /* XOR-in next input byte */
    byte data = (byte)(bytes[i] ^ crc);
    /* get current CRC value = remainder */
    crc = (byte)(crctable[data]);
  }
  return crc;
}


void setup() {
  DDRD |= (1 << PD1);
  delay(500);
  
  cli();
  
  UBRR1H = (unsigned char)(UBRRcalc>>8);
  UBRR1L = (unsigned char)(UBRRcalc);
 // дозвіл прийому-передачі+перерив.прийому, заверш.перед.+ 9n
   UCSR1B = (1<<UCSZ12)|(1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1);
     // формат кадру 9n2 без перевірки парності
    UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);
  //обнулення регістра UCSRA
  UCSR1A = 0;


  UBRR0H = (unsigned char)(UBRRcalc >> 8);
  UBRR0L = (unsigned char)(UBRRcalc);

  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
  // дозвіл прийому-передачі + перерив.прийому(регістр UCSRB)
  UCSR0A = 0; //скид прапорців регістра UCSRA

  sei();
  PORTD |= (1 << PD1);
}


void request(const byte data[3], bool address = true) {
    if (address) {
        UCSR1B |= (1 << TXB81);
    }
    else {
        UCSR1B &= ~(1 << TXB81);
    }
    UDR1 = data[0];
    for (int i = 1; i < 3; i++) {
      while ( !( UCSR1A & (1<<UDRE1)) );
      UCSR1B &= ~(1<< TXB81);
      UDR1 = data[i];
    }
    while ( !( UCSR1A & (1<<UDRE1)) );
    UDR1 = Compute_CRC8(data);
    digitalWrite(PD1, LOW);
    __asm__("nop\n\t");
}

ISR(USART0_RX_vect) // переривання при прийнятому байті даних
{
  byte packet[3];
  byte income = UDR0;
  if (byteCount < 3) {
    packet[byteCount++] = income;
  } else {
    if (Compute_CRC8(packet) == income) {
        byteCount = 0;
        packetAvailable = true;
        UCSR1B |= (1<< TXB81);
        if (packet[0] == slave1) {
            request(requestInfo1);
        } else if (packet[0] == slave2) {
            request(requestInfo2);
        }      
    } else {
        for (int i = 0; i < 3; i++) {
            while ( !( UCSR0A & (1 << UDRE0)) );
            UDR0 = errorOccured[i];
        }
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = Compute_CRC8(errorOccured);
        packetAvailable = false;
      /*byteCount = 0;
        for (int i =0; i<3; i++) {
        while ( !( UCSRnA & (1<<UDREn)) );
        UDR0 = errorOccured[i];
        }
        while ( !( UCSRnA & (1<<UDREn)) );
        UDR0 = Compute_CRC8(errorOccured);
        return;*/
    }
  }
}

ISR(USART1_RX_vect) {
    byte packet[3];
    byte income = UDR0;
    if (byteCount < 3) {
        packet[byteCount++] = income;
    } else {
        if (Compute_CRC8(packet) == income) {
            for (int i = 0; i < 3; i++) {
                QueuePut(packet[i]);
            }
            QueuePut(Compute_CRC8(packet));
        }
        else {
            dropped_packets[packet_counter] = 0xFF;
        }
        if (++packet_counter > 2) {
            __asm__("nop\n\t");
            for (int i = 0; i < 3; i++) {
                while (!(UCSR0A & (1 << UDRE0)));
                UDR0 = dropped_packets[i];
            }
            while (!(UCSR0A & (1 << UDRE0)));
            UDR0 = Compute_CRC8(dropped_packets);
        }
    }
}



void loop() {
    if (packet_counter == 3) {
        request(dropped_packets, false);
    }
    /*
    if (clean_package) {
        for (int i = 0; i < 3; i++) {
            while (!(UCSR0A & (1 << UDRE0)));
            UDR0 = errorOccured[i];
        }
    }*/
    while (QueueIn != QueueOut)
    {
        while (!(UCSR0A & (1 << UDRE0)));
        QueueGet();
    }
}
