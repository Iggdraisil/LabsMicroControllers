#include <Wire.h>
#include <stdio.h>
#include <math.h>


byte crctable[] = { 0x00, 0x1D, 0x3A, 0x27, 0x74, 0x69, 0x4E, 0x53, 0xE8, 0xF5, 0xD2, 0xCF, 0x9C, 0x81, 0xA6, 0xBB,
        0xCD, 0xD0, 0xF7, 0xEA, 0xB9, 0xA4, 0x83, 0x9E, 0x25, 0x38, 0x1F, 0x02, 0x51, 0x4C, 0x6B, 0x76,
        0x87, 0x9A, 0xBD, 0xA0, 0xF3, 0xEE, 0xC9, 0xD4, 0x6F, 0x72, 0x55, 0x48, 0x1B, 0x06, 0x21, 0x3C,
        0x4A, 0x57, 0x70, 0x6D, 0x3E, 0x23, 0x04, 0x19, 0xA2, 0xBF, 0x98, 0x85, 0xD6, 0xCB, 0xEC, 0xF1,
        0x13, 0x0E, 0x29, 0x34, 0x67, 0x7A, 0x5D, 0x40, 0xFB, 0xE6, 0xC1, 0xDC, 0x8F, 0x92, 0xB5, 0xA8,
        0xDE, 0xC3, 0xE4, 0xF9, 0xAA, 0xB7, 0x90, 0x8D, 0x36, 0x2B, 0x0C, 0x11, 0x42, 0x5F, 0x78, 0x65,
        0x94, 0x89, 0xAE, 0xB3, 0xE0, 0xFD, 0xDA, 0xC7, 0x7C, 0x61, 0x46, 0x5B, 0x08, 0x15, 0x32, 0x2F,
        0x59, 0x44, 0x63, 0x7E, 0x2D, 0x30, 0x17, 0x0A, 0xB1, 0xAC, 0x8B, 0x96, 0xC5, 0xD8, 0xFF, 0xE2,
        0x26, 0x3B, 0x1C, 0x01, 0x52, 0x4F, 0x68, 0x75, 0xCE, 0xD3, 0xF4, 0xE9, 0xBA, 0xA7, 0x80, 0x9D,
        0xEB, 0xF6, 0xD1, 0xCC, 0x9F, 0x82, 0xA5, 0xB8, 0x03, 0x1E, 0x39, 0x24, 0x77, 0x6A, 0x4D, 0x50,
        0xA1, 0xBC, 0x9B, 0x86, 0xD5, 0xC8, 0xEF, 0xF2, 0x49, 0x54, 0x73, 0x6E, 0x3D, 0x20, 0x07, 0x1A,
        0x6C, 0x71, 0x56, 0x4B, 0x18, 0x05, 0x22, 0x3F, 0x84, 0x99, 0xBE, 0xA3, 0xF0, 0xED, 0xCA, 0xD7,
        0x35, 0x28, 0x0F, 0x12, 0x41, 0x5C, 0x7B, 0x66, 0xDD, 0xC0, 0xE7, 0xFA, 0xA9, 0xB4, 0x93, 0x8E,
        0xF8, 0xE5, 0xC2, 0xDF, 0x8C, 0x91, 0xB6, 0xAB, 0x10, 0x0D, 0x2A, 0x37, 0x64, 0x79, 0x5E, 0x43,
        0xB2, 0xAF, 0x88, 0x95, 0xC6, 0xDB, 0xFC, 0xE1, 0x5A, 0x47, 0x60, 0x7D, 0x2E, 0x33, 0x14, 0x09,
        0x7F, 0x62, 0x45, 0x58, 0x0B, 0x16, 0x31, 0x2C, 0x97, 0x8A, 0xAD, 0xB0, 0xE3, 0xFE, 0xD9, 0xC4

};

const byte endMessage[]{ 0xF0, 0xF0, 0xF0 };
const byte errorOccured = 0xFF;
const byte ok_message = 0xC8;
const byte address = 0x37;
int accessCounter = 0;

const byte data[] = { 'O','l','e','h',' ','P'};
const unsigned int data_size = sizeof(data);
const int total_packets_count = data_size % 3 ? data_size / 3 + 1 : data_size / 3;
#define BAUD 9600
#define UBRRcalc 95  //(F_CPU/(BAUD*16L) - 1)

bool sending = false;
int byteCount = 0;
byte packet[3];
int tx_count = 0;
int packet_counter = -1;
byte packets[total_packets_count][3];
bool corrupted = false;
#define BMP180_ADDR 0x77 // 7-bit address
#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_RESULT 0xF6
#define BMP180_COMMAND_TEMPERATURE 0x2E
#define BMP180_COMMAND_PRESSURE 0x34 

uint16_t unSignedIntTempVar, AC4, AC5, AC6;
int16_t signedIntTempVar, AC1, AC2, AC3, VB1, VB2, MB, MC, MD;

double c5, c6, mc, md, x0, x1, x2, y0, y1, y2, p0, p1, p2;
double temp, pressure;
char _error;

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
    // put your setup code here, to run once:

    cli();
    UBRR0H = (unsigned char)(UBRRcalc >> 8);
    UBRR0L = (unsigned char)(UBRRcalc);
    // дозвіл прийому-передачі+перерив.прийому, заверш.перед.+ 9n
    UCSR0B = (1 << UCSZ02) | (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (1 << TXCIE0);
    // формат кадру 9n2 без перевірки парності
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    //обнулення регістра UCSRA
    UCSR0A = (1 << MPCM0);

    DDRD |= (1 << PD2);
    PORTD &= ~(1 << PD2);

    for (int i = 0; i < total_packets_count;) {
        for (int j = 0; j < 3; j++) {
            packets[i][j] = data[i * 3 + j];
        }
        i++;
    }
    sei();
    begin();
}

ISR(USART_RX_vect) {
    bool addr = bit_is_set(UCSR0B, RXB80);
    const byte income = UDR0;
    if (addr) {
        if (income == address) {
            UCSR0A &= ~(1 << MPCM0);
            short temperature = round(temp * 10);
            int32_t pressur = round(pressure * 100);
            char temperature_bytes[2] = { temperature, temperature >> 8 };
            char pressure_bytes[3] = { pressur >> 16, pressur >> 8 , pressur};
            for (int i = 0; i < 2; i++) {
                packets[0][i] = temperature_bytes[i];
            }
            packets[0][2] = 0x00;
            for (int i = 0; i < 3; i++) {
                packets[1][i] = pressure_bytes[i];
            }
            sending = true;
            packet_counter = 0;
        }
    } else {
        if (income == errorOccured) {
            packet_counter--;
            sending = true;
        } else if (income == ok_message){
            sending = true;
        }
    }

}

ISR(USART_TX_vect) {
    PORTD &= ~(1 << PD2);
}

void loop() {
    // put your main code here, to run repeatedly:
    if (sending && packet_counter > -1) {
        PORTD |= (1 << PD2);
        if (packet_counter < total_packets_count ) {
            loop_until_bit_is_set(PIND, PD2);
            for (int j = 0; j < 3; j++) {
                while (!(UCSR0A & (1 << UDRE0)));
                UCSR0B &= ~(1 << TXB80);
                UDR0 = packets[packet_counter][j];
            }
            while (!(UCSR0A & (1 << UDRE0)));
            UDR0 = Compute_CRC8(packets[packet_counter]);
            tx_count++;
            packet_counter++;
            sending = false;
        }
        else
        {
            packet_counter = -1;
            accessCounter++;
            for (int j = 0; j < 3; j++) {
                while (!(UCSR0A & (1 << UDRE0)));
                UCSR0B &= ~(1 << TXB80);
                UDR0 = endMessage[j];
            }
            while (!(UCSR0A & (1 << UDRE0)));
            UDR0 = Compute_CRC8(endMessage);
            sending = false;
        }
    }
    if (millis() % 1000 == 0) {
        measureParameters(pressure, temp);
    }
}

char begin() {
    Wire.begin();

    AC1 = readEEprom(0xAA, 1);
    AC2 = readEEprom(0xAC, 1);
    AC3 = readEEprom(0xAE, 1);
    AC4 = readEEprom(0xB0, 0);
    AC5 = readEEprom(0xB2, 0);
    AC6 = readEEprom(0xB4, 0);
    VB1 = readEEprom(0xB6, 1);
    VB2 = readEEprom(0xB8, 1);
    MB = readEEprom(0xBA, 1);
    MC = readEEprom(0xBC, 1);
    MD = readEEprom(0xBE, 1);

    double c3 = 160.0 * pow(2, -15) * AC3;
    double c4 = pow(10, -3) * pow(2, -15) * AC4;
    double b1 = pow(160, 2) * pow(2, -30) * VB1;
    c5 = (pow(2, -15) / 160) * AC5;
    c6 = AC6;
    mc = (pow(2, 11) / pow(160, 2)) * MC;
    md = MD / 160.0;
    x0 = AC1;
    x1 = 160.0 * pow(2, -13) * AC2;
    x2 = pow(160, 2) * pow(2, -25) * VB2;
    y0 = c4 * pow(2, 15);
    y1 = c4 * c3;
    y2 = c4 * b1;
    p0 = (3791.0 - 8.0) / 1600.0;
    p1 = 1.0 - 7357.0 * pow(2, -20);
    p2 = 3038.0 * 100.0 * pow(2, -36);

    return(1);
}


double readEEprom(char address, bool sign) {
    unsigned char data[2];
    char x;

    Wire.beginTransmission(BMP180_ADDR);
    Wire.write(address);
    _error = Wire.endTransmission();
    if (_error == 0) {
        Wire.requestFrom(BMP180_ADDR, 2);
        while (Wire.available() != 2);
        for (x = 0; x < 2; x++) {
            data[x] = Wire.read();
        }
        if (sign) {
            return signedIntTempVar = (int16_t)((data[0] << 8) | data[1]);
        }
        else {
            return unSignedIntTempVar = (((uint16_t)data[0] << 8) | (uint16_t)data[1]);
        }
    }
    signedIntTempVar = unSignedIntTempVar = 0;
    return(0);
}


char writeBytes(unsigned char* values, char length) {
    char x;

    Wire.beginTransmission(BMP180_ADDR);
    for (int i = 0; i < length; i++) {
        Wire.write(values[i]);
    }

    Wire.endTransmission();
}


char measureParameters(double& pressure_t, double& temp_t) {
    unsigned char data[3];
    double up, ut;

    data[0] = BMP180_REG_CONTROL;
    data[1] = BMP180_COMMAND_TEMPERATURE;
    writeBytes(data, 2);
    delay(5);

    Wire.beginTransmission(BMP180_ADDR);
    Wire.write(BMP180_REG_RESULT);

    if (Wire.endTransmission() == 0) {
        Wire.requestFrom(BMP180_ADDR, 2);
        while (Wire.available() != 2);
        for (int i = 0; i < 2; i++) {
            data[i] = Wire.read();
        }
        ut = (data[0] * 256.0) + data[1];
        temp_t = calculateTemperature(ut);
    }
    else {
        return(0);
    }

    data[0] = BMP180_REG_CONTROL;
    data[1] = BMP180_COMMAND_PRESSURE;
    writeBytes(data, 2);
    delay(26);

    Wire.beginTransmission(BMP180_ADDR);
    Wire.write(BMP180_REG_RESULT);

    if (Wire.endTransmission() == 0) {
        Wire.requestFrom(BMP180_ADDR, 3);
        while (Wire.available() != 3);
        for (int i = 0; i < 3; i++) {
            data[i] = Wire.read();
        }

        up = (data[0] * 256.0) + data[1] + (data[2] / 256.0);
        pressure_t = calculatePressure(up, temp_t);
    }
}

double calculateTemperature(double ut) {
    double T, a;

    a = c5 * (ut - c6);
    T = a + (mc / (a + md));

    return T;
}

double calculatePressure(double up, double T) {
    double P;
    double s, x, y, z;

    s = T - 25.0;
    x = (x2 * pow(s, 2)) + (x1 * s) + x0;
    y = (y2 * pow(s, 2)) + (y1 * s) + y0;
    z = (up - x) / y;
    P = (p2 * pow(z, 2)) + (p1 * z) + p0;

    return P;
}