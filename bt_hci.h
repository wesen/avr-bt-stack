#ifndef BT_HCI_H__
#define BT_HCI_H__

typedef enum {
   hci_cmd_pkt = 1,
   hci_acl_pkt,
   hci_sco_pkt,
   hci_evt_pkt
} hci_type_e;

typedef struct bt_acl_s {
   unsigned short handle;
   unsigned short pkt_types;
   unsigned char  max_slots;
} bt_acl_t;

/** size of a command packet header */
#define HCI_CMD_SIZE 3
/** size of a data packet header */
#define HCI_DATA_SIZE 4
/** size of a SCO packet header */
#define HCI_SCO_SIZE 3
/** size of an event packet header */
#define HCI_EVT_SIZE 2

void bt_hci_pack_cmd(bt_dev_t *dev, unsigned short ogf, 
                                    unsigned short ocf,
                                    unsigned char  len);
void bt_hci_pack_acl(bt_dev_t *dev, unsigned short handle,
                                    unsigned char  boundary,
                                    unsigned char  broadcast,
                                    unsigned char  len);
void bt_hci_pack_sco(bt_dev_t *dev, unsigned short handle,
                                    unsigned char  len);


bt_dev_evt_e bt_hci_unpack_hci(bt_dev_t *dev);

bt_dev_evt_e bt_hci_unpack_evt(bt_dev_t *dev);
bt_dev_evt_e bt_hci_unpack_cmd_status(bt_dev_t *dev, int len);
bt_dev_evt_e bt_hci_unpack_cmd_complete(bt_dev_t *dev, int len);
bt_dev_evt_e bt_hci_unpack_cc_read_local_features(bt_dev_t *dev);
bt_dev_evt_e bt_hci_unpack_cc_read_bd_addr(bt_dev_t *dev);

/** pack ogf and ocf into 16 bits HCI command opcode field */
#define CMD_OPCODE_PACK(ogf, ocf) (((ocf) & 0x03ff) | (((ogf) & 0x3f) << 10))
/** extract ogf from HCI command opcode field */
#define CMD_OPCODE_OGF(op) (((op) >> 10) & 0x3f)
/** extract ocf from HCI command opcode field */
#define CMD_OPCODE_OCF(op) ((op) & 0x03ff)

/** pack handle and flags into 16 bits HCI ACL handle field */
#define ACL_HANDLE_PACK(h, pb, bc) (((h) & 0x0fff) | ((bc)) << 14 | ((pb) << 12))
/** extract handle from HCI ACL handle field */
#define ACL_HANDLE(h) ((h) & 0x0fff)
/** extract flags from HCI ACL handle field */
#define ACL_FLAGS_PB(h) (((h) >> 12) & 0x3)
#define ACL_FLAGS_BC(h) (((h) >> 14) & 0x3)

#define HCI_ACL_CONT  0x01
#define HCI_ACL_START 0x02
#define HCI_ACL_LMP   0x03

#define HCI_ACL_PP    0x00
#define HCI_ACL_ABC   0x01
#define HCI_ACL_PBC   0x02

/* Bluetooth packet types */
#define HCI_DM1 0x0008
#define HCI_DM3 0x0400
#define HCI_DM5 0x4000
#define HCI_DH1 0x0010
#define HCI_DH3 0x0800
#define HCI_DH5 0x8000

#define HCI_HV1 0x0020
#define HCI_HV2 0x0040
#define HCI_HV3 0x0080

#define ACL_PTYPE_MASK (HCI_DM1 | HCI_DM3 | HCI_DM5 | \
                              HCI_DH1 | HCI_DH3 | HCI_DH5)
#define SCO_PTYPE_MASK (HCI_HV1 | HCI_HV2 | HCI_HV3)

/* HCI error codes */
#define HCI_SUCCESS 0x0 /** HCI command was successful */
#define HCI_REJECT_RESOURCES 0x0d
#define HCI_REJECT_SECURITY  0x0e
#define HCI_REJECT_PERSONAL  0x0f
#define HCI_DISCONNECT_USER  0x13

/* HCI COMMANDS */
#define OGF_LINK_CTL      0x01
#define OGF_LINK_POLICY   0x02
#define OGF_HOST_CTL      0x03
#define OGF_INFO_PARAM    0x04
#define OGF_STATUS_PARAM  0x05
#define OGF_VENDOR        0x3f

/* OGF_LINK_CTL */
#define OCF_CREATE_CONN         0x0005
#define OCF_DISCONNECT          0x0006
#define OCF_ADD_SCO             0x0007
#define OCF_ACCEPT_CONN_REQ     0x0009
#define OCF_REJECT_CONN_REQ     0x000a
#define OCF_LINK_KEY_REPLY      0x000b
#define OCF_LINK_KEY_REPLY_NEG  0x000c
#define OCF_PINCODE_REPLY       0x000d
#define OCF_PINCODE_REPLY_NEG   0x000e

/* OCF_INQUIRY */
#define OCF_INQUIRY             0x0001
#define INQUIRY_SIZE            5
/** General/Unlimited Inquiry Access Code */
#define GIAC_LAP                0x33, 0x8B, 0x9E
/** Limited Dedicated Inquiry Access Code */
#define LIAC_LAP                0x00, 0x8B, 0x9E

/* OGF_HOST_CTL */
#define OCF_RESET               0x0003
#define OCF_WRITE_AUTH_ENABLE   0x0020
#define OCF_WRITE_CA_TIMEOUT    0x0016
#define OCF_WRITE_PG_TIMEOUT    0x0018

#define OCF_READ_SCAN_ENABLE    0x0019
#define OCF_WRITE_SCAN_ENABLE   0x001A
#define INQUIRY_SCAN_ENABLE     0x1
#define PAGE_SCAN_ENABLE        0x2

#define OCF_SET_EVENT_FLT       0x0005
#define OCF_CHANGE_LOCAL_NAME   0x0013
#define OCF_READ_LOCAL_NAME     0x0014
#define OCF_READ_CLASS_OF_DEV   0x0023
#define OCF_WRITE_CLASS_OF_DEV  0x0024

#define OCF_READ_AUTH_ENABLE    0x001f
#define OCF_WRITE_AUTH_ENABLE   0x0020

/* OGF_INFO_PARAM */
#define OCF_READ_LOCAL_VERSION  0x0001
#define OCF_READ_LOCAL_FEATURES 0x0003
#define OCF_READ_BUFFER_SIZE    0x0005
#define OCF_READ_BD_ADDR        0x0009

/* OGF_VENDOR */
#ifdef BTERICSSON
#define OCF_ERICSSON_READ_MEM   0x0001
#define OCF_ERICSSON_WRITE_MEM  0x0002
#define OCF_ERICSSON_READ_REG   0x0003
#define OCF_ERICSSON_WRITE_REG  0x0004
#define OCF_ERICSSON_READ_I2C   0x0005
#define OCF_ERICSSON_WRITE_I2C  0x0006
#define OCF_ERICSSON_WRITE_PCM_SETTINGS \
                                0x0007
#define OCF_ERICSSON_SET_BAUDRATE \
                                0x0009
#define OCF_ERICSSON_WRITE_COUNTRY_CODE \
                                0x000c
#define OCF_ERICSSON_READ_REV   0x000f
#define OCF_ERICSSON_SELF_TEST  0x0010
#define OCF_ERICSSON_ENTER_TEST 0x0011
#define OCF_ERICSSON_TEST_CTL   0x0012
#define OCF_ERICSSON_AUX1       0x0013
#define OCF_ERICSSON_BER        0x0015
#define OCF_ERICSSON_PPM        0x0017
#define OCF_ERICSSON_EXIT_PPM   0x0018
#define OCF_ERICSSON_TX_TEST    0x0019
#endif /* BTERICSSON */

/* OCF_READ_LOCAL_VERSION */
#define READ_LOCAL_VERSION_SIZE 9

/* OCF_READ_LOCAL_FEATURES */
#define OCF_READ_LOCAL_FEATURES_SIZE 9

#define LMP_3SLOT     0x01
#define LMP_5SLOT     0x02
#define LMP_ENCRYPT   0x04
#define LMP_SOFFSET   0x08
#define LMP_TACCURACY 0x10
#define LMP_RSWITCH   0x20
#define LMP_HOLD      0x40
#define LMP_SNIF      0x80

#define LMP_PARK      0x01
#define LMP_RSSI      0x02
#define LMP_QUALITY   0x04
#define LMP_SCO       0x08
#define LMP_HV2       0x10
#define LMP_HV3       0x20
#define LMP_ULAW      0x40
#define LMP_ALAW      0x80


/* OCF_READ_BUFFER_SIZE */
#define OCF_READ_BUFFER_SIZE_SIZE 8

/* OCF_READ_BD_ADDR */
#define OCF_READ_BD_ADDR_SIZE 7

#ifdef BTERICSSON
#define BTERICSSON_B460800 0x00
#define BTERICSSON_B230400 0x01
#define BTERICSSON_B115200 0x02
#define BTERICSSON_B57600  0x03
#define BTERICSSON_B28800  0x04
#define BTERICSSON_B14400  0x05
#define BTERICSSON_B7200   0x06
#define BTERICSSON_B3600   0x07
#define BTERICSSON_B1800   0x08
#define BTERICSSON_B900    0x09

#define BTERICSSON_B153600 0x10
#define BTERICSSON_B76800  0x11
#define BTERICSSON_B38400  0x12
#define BTERICSSON_B19200  0x13
#define BTERICSSON_B9600   0x14
#define BTERICSSON_B4800   0x15
#define BTERICSSON_B2400   0x16
#define BTERICSSON_B1200   0x17
#define BTERICSSON_B600    0x18
#define BTERICSSON_B300    0x19
#endif /* BTERICSSON */

/* OCF_SET_EVENT_FLT */
#define FLT_CLEAR_ALL           0x00

/* HCI EVENTS */

#define BTEVT_INQUIRY_COMPLETE 0x01
#define BTEVT_INQUIRY_RESULT   0x02
#define BTEVT_CONN_COMPLETE    0x03
#define BTEVT_CONN_REQUEST     0x04
#define BTEVT_DISCONN_COMPLETE 0x05
#define BTEVT_AUTH_COMPLETE    0x06
#define BTEVT_CMD_COMPLETE     0x0e
#define BTEVT_CMD_STATUS       0x0f
#define BTEVT_NUM_COMP_PKTS    0x13
#define BTEVT_MAX_SLOTS_CHG    0x1b
#define BTEVT_HCI_DEV_EVENT    0xfd
#define BTEVT_READ_FEATURES    0x0b
#define BTEVT_PINCODE_REQ      0x16
#define BTEVT_LINK_KEY_REQ     0x17
#define BTEVT_LINK_KEY_NOT     0x18

/* BTEVT_INQUIRY_RESULT */
#define INQUIRY_RESULT_SIZE 14

#endif /* BT_HCI_H__ */
