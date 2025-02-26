#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define HEX_DATA_LEN_MAX 50

typedef struct {
    uint32_t addr;
    uint8_t* data;
    uint8_t  len;
} HEX_DATA_t;

typedef struct {
    uint8_t buf[2 * HEX_DATA_LEN_MAX];
    uint8_t count;
    uint8_t state;
    uint8_t addr_h[4];

    uint8_t header;
    uint8_t len;
    uint8_t addr_l[4];
    uint8_t type;
    uint8_t data[2 * HEX_DATA_LEN_MAX];
    uint8_t checksum;
} HEX_OBJ_t;

#define HEX_E_OK     0
#define HEX_E_WAIT   1
#define HEX_E_FINISH 2
#define HEX_E_ERROR  -1

extern HEX_OBJ_t* hex_newobject(void);
extern void       hex_resetobject(HEX_OBJ_t* obj);
extern uint8_t    hex_findobject(HEX_OBJ_t* obj, uint8_t ch);
extern uint8_t    hex_getdata(HEX_OBJ_t* obj, HEX_DATA_t* data);

#define _ASCII_2_BYTE(ch1, ch2) ((lookuptable[(ch1) - '0'] << 4) | (lookuptable[(ch2) - '0']))

static const uint8_t lookuptable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, ':', ';', '<', '=', '>', '?', '@', 10, 11, 12, 13, 14, 15};
static HEX_OBJ_t     HexObj;

static uint8_t hex_ascii2byte(uint8_t* ascii, uint8_t* bytes, uint8_t len)
{
    uint8_t i = 0;

    if (len % 2)
    {
        return 0;
    }

    for (i = 0; i < len / 2; i++)
    {
        bytes[i] = _ASCII_2_BYTE(ascii[2 * i], ascii[2 * i + 1]);
    }

    return len / 2;
}

HEX_OBJ_t* hex_newobject(void)
{
    return &HexObj;
}

void hex_resetobject(HEX_OBJ_t* obj)
{
    memset(obj, 0, sizeof(HEX_OBJ_t));
}

uint8_t hex_findobject(HEX_OBJ_t* obj, uint8_t ch)
{
    uint8_t result = HEX_E_WAIT;

    if (NULL == obj)
    {
        return HEX_E_ERROR;
    }

    switch (obj->state)
    {
        case 0:  // search header
        {
            if (':' == ch)
            {
                // hex_resetobject(obj);
                obj->state  = 1;
                obj->header = ch;
                obj->count  = 2;
            }
            break;
        }
        case 1:  // len
        {
            obj->buf[2 - obj->count] = ch;
            if (--obj->count == 0)
            {
                obj->len = _ASCII_2_BYTE(obj->buf[0], obj->buf[1]);

                if (0 == obj->len)  // File Ends
                {
                    obj->buf[0] = '0';
                    obj->buf[1] = '0';
                    obj->buf[2] = '0';
                    obj->buf[3] = '0';
                    obj->buf[4] = '0';
                    obj->buf[5] = '1';
                    obj->buf[6] = 'F';
                    obj->buf[7] = 'F';
                    obj->state  = 8;
                    obj->count  = 8;
                }
                else if (obj->len <= HEX_DATA_LEN_MAX)
                {
                    obj->count = 4;
                    obj->state = 2;
                }
                else  // Error
                {
                    hex_resetobject(obj);
                    result = HEX_E_ERROR;
                }
            }
            break;
        }
        case 2:  // address
        {
            obj->buf[4 - obj->count] = ch;
            if (--obj->count == 0)
            {
                obj->addr_l[0] = _ASCII_2_BYTE(obj->buf[0], obj->buf[1]);
                obj->addr_l[1] = _ASCII_2_BYTE(obj->buf[2], obj->buf[3]);
                obj->state     = 3;
                obj->count     = 2;
            }
            break;
        }
        case 3:  // type
        {
            obj->buf[2 - obj->count] = ch;
            if (--obj->count == 0)
            {
                // Hex Line Type
                obj->type = _ASCII_2_BYTE(obj->buf[0], obj->buf[1]);

                switch (obj->type)
                {
                    case 0x00:  // data
                    case 0x04:  // Section
                    case 0x05:  // Donothing
                    {
                        obj->state = 4;
                        obj->count = obj->len * 2;
                    }
                    break;

                    default:
                    {
                        hex_resetobject(obj);
                        result = HEX_E_ERROR;
                    }
                    break;
                }
            }
            break;
        }
        case 4:  // data
        {
            obj->buf[obj->len * 2 - obj->count] = ch;
            if (--obj->count == 0)
            {
                hex_ascii2byte(obj->buf, obj->data, obj->len * 2);

                if (0x04 == obj->type)
                {
                    memcpy(obj->addr_h, obj->data, obj->len);
                }
                else if (0x05 == obj->type)
                {
                    // Donothing
                }

                obj->state = 5;
                obj->count = 2;
            }
            break;
        }
        case 5:  // checksum
        {
            obj->buf[2 - obj->count] = ch;

            if (--obj->count == 0)
            {
                uint8_t sum   = 0;
                obj->checksum = _ASCII_2_BYTE(obj->buf[0], obj->buf[1]);

                sum += obj->len;
                sum += obj->addr_l[0];
                sum += obj->addr_l[1];
                sum += obj->type;

                {
                    uint8_t i = 0;
                    for (i = 0; i < obj->len; i++)
                    {
                        sum += obj->data[i];
                    }
                }

                sum += obj->checksum;

                if (0 == sum)  // Passed
                {
                    obj->state = 0;
                    result     = HEX_E_OK;
                }
                else  // Failured
                {
                    hex_resetobject(obj);
                    result = HEX_E_ERROR;
                }
            }
            break;
        }

        case 8:  // File End Sqeue
        {
            uint8_t i = 8 - obj->count;
            if (obj->buf[i] == ch)
            {
                if (--obj->count == 0)
                {
                    hex_resetobject(obj);
                    result = HEX_E_FINISH;
                }
            }
            else
            {
                hex_resetobject(obj);
                result = HEX_E_ERROR;
            }
        }
        break;
    }

    return result;
}

uint8_t hex_getdata(HEX_OBJ_t* obj, HEX_DATA_t* data)
{
    uint8_t* p = (uint8_t*)&(data->addr);

    if (obj->type)
    {
        return 0;
    }

    p[0] = obj->addr_l[1];
    p[1] = obj->addr_l[0];
    p[2] = obj->addr_h[1];
    p[3] = obj->addr_h[0];

    data->data = obj->data;
    data->len  = obj->len;

    return data->len;
}

const char hexdata[] = ":020000040800F2\
:10000000080A00204501000861030008F902000801 \
:100010005D030008C7010008D504000800000000C7 \
:100020000000000000000000000000008503000840 \
:10003000CB01000800000000630300087104000801 \
:100040005F0100085F0100085F0100085F01000810 \
:100050005F0100085F0100085F0100085F01000800 \
:100060005F0100085F0100085F0100085F010008F0 \
:100070005F0100085F0100085F0100085F010008E0 \
:100080005F0100085F0100085F0100085F010008D0 \
:100090005F0100085F0100085F0100085F010008C0 \
:1000A0005F0100085F0100085F0100085F010008B0 \
:1000B0005F0100085F0100085F0100085F010008A0 \
:1000C0005F0100085F0100085F0100085F01000890 \
:1000D0005F0100085F0100085F0100085F01000880 \
:1000E0005F0100085F0100085F0100085F01000870 \
:1000F0005F0100085F0100085F0100085F01000860 \
:100100005F0100085F0100085F0100085F0100084F \
:100110005F0100085F0100085F0100085F0100083F \
:100120005F0100085F0100085F0100085F0100082F \
:10013000DFF80CD000F018F8004800474905000827 \
:10014000080A00200648804706480047FEE7FEE709 \
:10015000FEE7FEE7FEE7FEE7FEE7FEE7FEE7FEE777 \
:100160007504000831010008064C074D06E0E06800 \
:1001700040F0010394E8070098471034AC42F6D3EE \
:10018000FFF7DAFFF40600081407000830B58C18F2 \
:1001900010F8012B12F00F0301D110F8013B1209E6 \
:1001A00006D110F8012B03E010F8015B01F8015BA8 \
:1001B0005B1EF9D101E001F8013B521EFBD1A142C7 \
:1001C000E6D3002030BD00BFFEE7704701B502E076 \
:1001D0000098401E009000980028F9D108BD2DE934 \
:1001E000F0410246002500260020002300240027BD \
:1001F00091F803C00CF00F0591F803C00CF0100C3F \
:10020000BCF1000F03D091F802C04CEA050591F84B \
:1002100000C0BCF1000F31D0146800202BE04FF07B \
:10022000010C0CFA00F3B1F800C00CEA03069E4280 \
:1002300020D183004FF00F0C0CFA03F7BC4305FAF2 \
:1002400003FC4CEA040491F803C0BCF1280F06D16A \
:100250004FF0010C0CFA00FCC2F814C00AE091F84F \
:1002600003C0BCF1480F05D14FF0010C0CFA00FCA3 \
:10027000C2F810C0401C0828D1D31460B1F800C0E7 \
:10028000BCF1FF0F34DD546800202EE000F1080CB3 \
:100290004FF0010808FA0CF3B1F800C00CEA0306AD \
:1002A0009E4221D183004FF00F0C0CFA03F7BC43A0 \
:1002B00005FA03FC4CEA040491F803C0BCF1280FD2 \
:1002C00005D100F1080C08FA0CF8C2F8148091F876 \
:1002D00003C0BCF1480F07D100F1080C4FF0010832 \
:1002E00008FA0CF8C2F81080401C0828CED35460DD \
:1002F000BDE8F0810161704700BFFEE708B501214C \
:10030000082000F02FF82020ADF8000010208DF814 \
:10031000030003208DF8020069460F48FFF75FFFD6 \
:100320000120ADF8000069460B48FFF758FF022096 \
:10033000ADF8000069460848FFF751FF2021064844 \
:10034000FFF7D8FF01210448FFF7D4FF022102483C \
:10035000FFF7D0FF08BD0000000C014000BFFEE722 \
:100360007047704729B1064A92690243044B9A616B \
:1003700004E0034A92698243014B9A61704700008E \
:1003800000100240704710B500F002F810BD0000E8 \
:100390000CB50020019000903348006840F4803094 \
:1003A0003149086000BF3048006800F40030009018 \
:1003B0000198401C0190009818B90198B0F5A06F01 \
:1003C000F1D12948006800F4003010B101200090FC \
:1003D00001E0002000900098012843D123480068E4 \
:1003E00040F01000214908600846006820F0030032 \
:1003F00008600846006840F0020008601A4840683B \
:10040000194948600846406848600846406840F41A \
:10041000806048600846406820F47C1048600846C8 \
:10042000406840F4E81048600846006840F080707A \
:10043000086000BF0C48006800F000700028F9D088 \
:100440000948406820F003000749486008464068B2 \
:1004500040F00200486000BF0348406800F00C0014 \
:100460000828F9D10CBD0000001002400020024015 \
:100470007047000010B51348006840F001001149B2 \
:10048000086008464068104908400E494860084620 \
:1004900000680E4908400B4908600846006820F4CF \
:1004A000802008600846406820F4FE0048604FF451 \
:1004B0001F008860FFF767FF4FF000600449086085 \
:1004C00010BD0000001002400000FFF8FFFFF6FE24 \
:1004D00008ED00E000BFFEE702E008C8121F08C1F7 \
:1004E000002AFAD170477047002001E001C1121FB5 \
:1004F000002AFBD17047000010B5002821D0114B15 \
:100500000022001F196803E0814203D80A464968A7 \
:100510000029F9D152B11368841A9C4204D10068B1 \
:1005200018441060104602E0506000E0186039B1D5 \
:1005300002680B1A934203D10B681A44026049689F \
:10054000416010BD000400208EB0FFF7D7FE3220BE \
:1005500000F09AF8044620780A3020700A208DF8BE \
:100560002C00207850B99DF82C000B2806D1434868 \
:10057000807A642802D12020414908602046FFF794 \
:10058000BBFF79E020203E4908603E48FFF71EFE91 \
:1005900020203B49091F08600120091D08603948D7 \
:1005A000FFF714FE01203649091F08600220091DCB \
:1005B00008603448FFF70AFE02203149091F08602D \
:1005C0002020091D08600120091F08600220086022 \
:1005D0002C48FFF7FBFD20202949091F0860012056 \
:1005E000091D08600220091F08602648FFF7EEFD7C \
:1005F00020202349091F0860012008600220091DEE \
:1006000008602048FFF7E2FD20201D490860012016 \
:1006100008600220091F08601A48FFF7D7FD202054 \
:10062000174908600120091F08600220091D0860A1 \
:100630001448FFF7CBFD20201149091F0860012055 \
:10064000091D0860022008600E48FFF7BFFD20204A \
:100650000B49086001200860022008600948FFF784 \
:10066000B5FD20200649091F08600120086002200E \
:1006700008600448FFF7AAFD84E70000000000209E \
:10068000140C0140FFFF0F00F0B50B30154D20F0AA \
:1006900007010024144A0FE003688B420BD38B42FE \
:1006A00005D95E1A43184768C3E9006700E043684C \
:1006B000136002C004E0021D10680028ECD120463F \
:1006C000002802D1286808B10020F0BD064A07487A \
:1006D0001060074A121A22F00702C0E90024012024 \
:1006E0002860D7E704040020000400200C04002048 \
:1006F000080600201407000800000020080400007D \
:100700008C010008240700080804002000060000EF \
:10071000E804000808FF0001020304380501FF0196 \
:04072000FF01FFA135 \
:0400000508000131BD \
:00000001FF";

int main()  // 和 j-flash 解析一致
{
    HEX_OBJ_t* mHex = hex_newobject();

    uint16_t i;
    uint8_t  result;

    for (i = 0; i < sizeof(hexdata); i++)
    {
        result = hex_findobject(mHex, hexdata[i]);

        if (HEX_E_OK == result)
        {
            HEX_DATA_t mData;

            if (hex_getdata(mHex, &mData))
            {
                uint16_t* p   = (uint16_t*)mData.data;
                uint16_t  len = (uint16_t)(mData.len / 2);

                for (uint16_t j = 0; j < len; j++)
                {
                    printf("%04X ", p[j]);
                }

                printf("\n");
            }
        }
        else if (HEX_E_FINISH == result)
        {
            printf("ok");
            break;
        }
    }

    return 0;
}
