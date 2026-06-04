# lwip ping 电脑
需要修改ethernetif.c文件，使用cube mxv6.16.0生成的eth_custom_phy_interface接口
同时cube mx不能自动生成void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)和void HAL_ETH_MspDeInit(ETH_HandleTypeDef* heth)，需要在stm32f4xx_hal_msp.c文件中实现，还需要在 MX_GPIO_Init() 和所有外设初始化完成之后、osKernelInitialize() 之前调用 ETH_PHY_Reset()，此函数也需要自己写。

