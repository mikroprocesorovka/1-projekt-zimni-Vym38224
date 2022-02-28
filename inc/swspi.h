#define DIN_GPIO GPIOB
#define DIN_PIN GPIO_PIN_3
#define CS_GPIO GPIOB
#define CS_PIN GPIO_PIN_4
#define CLK_GPIO GPIOB
#define CLK_PIN GPIO_PIN_5


void swspi_init(void);
void swspi_tx16(uint16_t data);
void swspi_adressXdata(uint8_t adress, uint8_t data);
void swspi_send_number(uint32_t number);
void swspi_send_time (uint8_t seconds,uint8_t minutes, uint8_t hours);
void swspi_toggle(uint8_t rad, uint8_t hodnota_radu);
void swspi_toggle_slowly(uint8_t rad, uint8_t hodnota_radu);