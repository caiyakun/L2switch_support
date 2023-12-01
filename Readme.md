说明：

    -usbserial_ctrl.py 配合当前branch下的main component使用
    -local测试时，使用IDFV5.2环境
    -读mac寄存器请调用reg_rd函数
    -写mac寄存器请调用reg_wr函数
    -读phy寄存器请调用reg_rd_phy函数
    -写phy寄存器请调用reg_wr_phy函数
    -device address是switch的地址，固定为0x73, Diff from phy(device) address

    -----------------------------------------
        example:
        // 读mac寄存器0x00060000 - 带返回值
        phy.reg_rd(0x00060000)  
        or
        value = phy.reg_rd(0x00060000) 

        //写mac寄存器0x00060000
        phy.reg_wr(0x00060000,0x001F0001)

        // 读phy0的0x2寄存器
        phy.reg_rd_phy(0x00,0x02) # default 0x0007

        // 写phy0的0x1寄存器
        phy.reg_wr_phy(0x00,0x01,0xXXXX)
    ------------------------------------------

MDIO：

    SWITCH通过IIC配置SMA模块来控制MDIO，从而配置PHY寄存器
    emac[i]内部包含 SMA(station management模块)，通过MDIO访问外部PHY设备.
    EMAC0 base_addr = xxx6_2000
    EMAC1 base_addr = xxx6_4000
    EMAC2 base_addr = xxx6_6000
    EMAC3 base_addr = xxx6_8000
    EMAC4 base_addr = xxx6_a000


    EMAC_MDIO_ADDRESS_REG/      base_addr + 0x40
    bit[15:11]/                 phy设备 device 地址
    bit[10:6]/                  phy设备内部reg_addr地址
    bit[5:2]/reg_appclkrange    分频选择MDIO/MDC 时钟 ,配置 0011b  ；//sys_clk 40MHz
    bit[1]/reg_smard.           写：0, 读：1
    bit[0]/reg_busy_bit         1b表示SMA正在操作MDIO，0b表示SMA操作MDIO完成
    
    EMAC_MDIO_DATA_REG/         base_addr + 0x44
    bit[31:16]/reg_rdata:       phy设计读返回rdata
    bit[15: 0]/reg_smadata:     phy设备内部reg寄存器 写wdata数据


PHY Addr (Depends on Hardware strap resistor,pls refer phy spec)

    emac0 -> 0x11 (phy0) - fpga on board phy - dp83867is
    emac1 -> 0x00 (phy1) - external phy VSC8541-01
    emac2 -> 0x01 (phy2) - external phy VSC8541-01
    emac3 -> 0x02 (phy3) - external phy VSC8541-01
    emac4 -> 0x04 (phy4) - external phy VSC8541-01

配置内部寄存器之前需要打开 MAC 时钟和软复位:

    phy.reg_wr(0x00060000,0x001F0001)
    phy.reg_wr(0x00060004,0x001F001F)
    phy.reg_wr(0x00060008,0x001F001F)
    phy.reg_wr(0x0006000c,0x001F001F)



Deprecated

    需要打开debug后方可访问mem
    ```
    // 打开debug
    -device=0x73 -reg=0x00041034 -wdata=0x20220801 -w=1
    -device=0x73 -reg=0x00041030 -wdata=0x0000ffff -w=1

    // 写
    -device=0x73 -reg=0x00072040 -wdata=0x12345678 -w=1
    -device=0x73 -reg=0x00072044 -wdata=0x12345678 -w=1

    // 读
    -device=0x73 -reg=0x00072040 -wdata=0x00000000 -w=0
    -device=0x73 -reg=0x00072044 -wdata=0x00000000 -w=0

    ```





