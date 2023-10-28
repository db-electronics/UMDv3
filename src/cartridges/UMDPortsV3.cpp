#include "cartridges/UMDPortsV3.h"

/////////////////////////////////////////////////
// PUBLIC
/////////////////////////////////////////////////

inline void UMDPortsV3::waitNs(uint16_t nanoSeconds){
    TIM7->CNT = 0;
    ticks = nanoSeconds/(1000/168);
    while(TIM7->CNT <= ticks);
}

void UMDPortsV3::setDefaults(){

    // setup timer 7 for IO timing
    TIM_HandleTypeDef htim7;
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    __HAL_RCC_TIM7_CLK_ENABLE();
    htim7.Instance = TIM7;
    htim7.Init.Prescaler = 1;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 65535;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim7);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig);

    // setup GPIO
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // enable master clock output on MCo1 PA8
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_4);

    //A0 to A7 on PA0 to PA
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //A8 to A15 on PC0 to PC7
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    //A16 to A23 on PD8 to PD15
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    //D0 to D15 on PE0 to PE15
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    //CE0 on PC13
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = UMD_PIN_CE0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(UMD_PORT_CE0, &GPIO_InitStruct);
    this->_portBitSet(UMD_PORT_CE0, UMD_PIN_CE0);
    //GPIOB->BSRR |= (1<<7);

    //CE1 - CE3 on PD5, PD6, PD7
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    this->_portBitSet(UMD_PORT_CE1, UMD_PIN_CE1);
    this->_portBitSet(UMD_PORT_CE2, UMD_PIN_CE2);
    this->_portBitSet(UMD_PORT_CE3, UMD_PIN_CE3);

    // RD and WR on PC14, PC15
    GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/////////////////////////////////////////////////
// PROTECTED
/////////////////////////////////////////////////
void UMDPortsV3::addressWrite(uint16_t address){
    this->_portByteWriteLow(UMD_PORT_ADDR_LOW, (uint8_t)(0xFF & address));
    this->_portByteWriteLow(UMD_PORT_ADDR_MID, (uint8_t)(0xFF & address>>8));
}

void UMDPortsV3::addressWrite(uint32_t address){
    this->_portByteWriteLow(UMD_PORT_ADDR_LOW, (uint8_t)(0xFF & address));
    this->_portByteWriteLow(UMD_PORT_ADDR_MID, (uint8_t)(0xFF & address>>8));
    this->_portByteWriteHigh(UMD_PORT_ADDR_HIGH, (uint8_t)(0xFF & address>>16));
}

uint8_t UMDPortsV3::dataReadLow(){
    return this->_portByteReadLow(UMD_PORT_DATA);
}

uint8_t UMDPortsV3::dataReadHigh(){
    return this->_portByteReadHigh(UMD_PORT_DATA);
}

uint16_t UMDPortsV3::dataReadWord(){
    return this->_portWordRead(UMD_PORT_DATA);
}

uint16_t UMDPortsV3::dataReadWordSwapped(){
    return UMD_PORT_SWAP_BYTES(this->_portWordRead(UMD_PORT_DATA));
}

void UMDPortsV3::dataWrite(uint8_t value){
    this->_portByteWriteLow(UMD_PORT_DATA, value);
}

void UMDPortsV3::dataWriteLow(uint8_t value){
    this->_portByteWriteLow(UMD_PORT_DATA, value);
}

void UMDPortsV3::dataWriteHigh(uint8_t value){
    this->_portByteWriteHigh(UMD_PORT_DATA, value);
}

void UMDPortsV3::dataWrite(uint16_t value){
    this->_portWordWrite(UMD_PORT_DATA, value);
}

void UMDPortsV3::dataWriteSwapped(uint16_t value){
    this->_portWordWrite(UMD_PORT_DATA, UMD_PORT_SWAP_BYTES(value));
}

void UMDPortsV3::dataSetToInputs(bool pullups){
    this->_portSetToInput(UMD_PORT_DATA, pullups);
}
void UMDPortsV3::dataSetToOutputs(){
    this->_portSetToOutput(UMD_PORT_DATA);
}

void UMDPortsV3::setCE0(){
    this->_portBitSet(UMD_PORT_CE0, UMD_PIN_CE0);
}

void UMDPortsV3::setCE1(){
    this->_portBitSet(UMD_PORT_CE1, UMD_PIN_CE1);
}

void UMDPortsV3::setCE2(){
    this->_portBitSet(UMD_PORT_CE2, UMD_PIN_CE2);
}

void UMDPortsV3::setCE3(){
    this->_portBitSet(UMD_PORT_CE3, UMD_PIN_CE3);
}

void UMDPortsV3::setRD(){
    this->_portBitSet(UMD_PORT_RD, UMD_PIN_RD);
}

void UMDPortsV3::setWR(){
    this->_portBitSet(UMD_PORT_WR, UMD_PIN_WR);
}

void UMDPortsV3::clearCE0(){
    this->_portBitClear(UMD_PORT_CE0, UMD_PIN_CE0);
}

void UMDPortsV3::clearCE1(){
    this->_portBitClear(UMD_PORT_CE1, UMD_PIN_CE1);
}

void UMDPortsV3::clearCE2(){
    this->_portBitClear(UMD_PORT_CE2, UMD_PIN_CE2);
}

void UMDPortsV3::clearCE3(){
    this->_portBitClear(UMD_PORT_CE3, UMD_PIN_CE3);
}

void UMDPortsV3::clearRD(){
    this->_portBitClear(UMD_PORT_RD, UMD_PIN_RD);
}

void UMDPortsV3::clearWR(){
    this->_portBitClear(UMD_PORT_WR, UMD_PIN_WR);
}

/////////////////////////////////////////////////
// PRIVATE
/////////////////////////////////////////////////
void UMDPortsV3::_portBitSet(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
    GPIOx->BSRR = GPIO_Pin;
}

void UMDPortsV3::_portBitClear(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
    GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U;
}

void UMDPortsV3::_portByteWriteLow(GPIO_TypeDef *GPIOx, uint8_t value){
    uint16_t portValue = GPIOx->IDR;
    portValue &= 0xFF00;
    portValue |= (uint16_t)(value);
    GPIOx->ODR = portValue;
}

uint8_t UMDPortsV3::_portByteReadLow(GPIO_TypeDef *GPIOx){
    return (uint8_t)(GPIOx->IDR & 0xFF);
}

void UMDPortsV3::_portByteWriteHigh(GPIO_TypeDef *GPIOx, uint8_t value){
    uint16_t portValue = GPIOx->IDR;
    portValue &= 0x00FF;
    portValue |= (uint16_t)(value<<8);
    GPIOx->ODR = portValue;
}

uint8_t UMDPortsV3::_portByteReadHigh(GPIO_TypeDef *GPIOx){
    return (uint8_t)((GPIOx->IDR>>8) & 0xFF);
}

void UMDPortsV3::_portWordWrite(GPIO_TypeDef *GPIOx, uint16_t value){
    GPIOx->ODR = value;
}

uint16_t UMDPortsV3::_portWordRead(GPIO_TypeDef *GPIOx){
    return (uint16_t)GPIOx->IDR;
}

void UMDPortsV3::_portSetToInput(GPIO_TypeDef *GPIOx, bool pullups){
    //D0 to D15 on PE0 to PE15
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    if(pullups){
      GPIO_InitStruct.Pull = GPIO_PULLUP;
    }else{
      GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void UMDPortsV3::_portSetToOutput(GPIO_TypeDef *GPIOx){
    //D0 to D15 on PE0 to PE15
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}
