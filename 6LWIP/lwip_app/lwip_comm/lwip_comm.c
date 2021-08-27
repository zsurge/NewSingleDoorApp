
#define LOG_TAG    "LWIP_COMM"
#include "elog.h"

#include "lan8720.h" 
#include "lwip_comm.h"

#include	"LwipDHCP_Task.h"
#include	"LwipCom_Task.h"

#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "ethernetif.h"
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h"
#include "lwip/timers.h"
#include "malloc.h"
#include "delay.h"
#include <stdio.h>
#include <time.h>
#include "tool.h"
#include "eth_cfg.h"
#include "led_task.h"



__lwip_dev lwipdev;						//lwip控制结构体

struct netif lwip_netif;				//定义一个全局的网络接口

static LinkState_TypeDef GB_LinkState = Link_Down; 
void ETH_link_callback(struct netif *netif);


extern u32 memp_get_memorysize ( void );	//在memp.c里面定义
extern u8_t* memp_memory;				//在memp.c里面定义.
extern u8_t* ram_heap;					//在mem.c里面定义.

//用于以太网中断调用
void lwip_pkt_handle ( void )
{
	ethernetif_input ( &lwip_netif );
}

//lwip中mem和memp的内存申请
//返回值:0,成功;
//    其他,失败
u8 lwip_comm_mem_malloc ( void )
{
//	u32 mempsize;
//	u32 ramheapsize;
//	mempsize=memp_get_memorysize();			//得到memp_memory数组大小
//	memp_memory=mymalloc ( SRAMIN,mempsize );	//为memp_memory申请内存
//	ramheapsize=LWIP_MEM_ALIGN_SIZE ( MEM_SIZE )+2*LWIP_MEM_ALIGN_SIZE ( 4*3 )+MEM_ALIGNMENT; //得到ram heap大小
//	ram_heap=mymalloc ( SRAMIN,ramheapsize );	//为ram_heap申请内存
//	if ( !memp_memory||!ram_heap ) //有申请失败的
//	{
//		lwip_comm_mem_free();
//		return 1;
//	}
//	return 0;

u32 mempsize;
u32 ramheapsize;
mempsize=memp_get_memorysize();         //得到memp_memory数组大小
memp_memory=mymalloc ( SRAMCCM,mempsize );   //为memp_memory申请内存
ramheapsize=LWIP_MEM_ALIGN_SIZE ( MEM_SIZE )+2*LWIP_MEM_ALIGN_SIZE ( 4*3 )+MEM_ALIGNMENT; //得到ram heap大小
ram_heap=mymalloc ( SRAMCCM,ramheapsize );   //为ram_heap申请内存
if ( !memp_memory||!ram_heap ) //有申请失败的
{
    lwip_comm_mem_free();
    return 1;
}
return 0;

}
//lwip中mem和memp内存释放
void lwip_comm_mem_free ( void )
{
//	myfree ( SRAMIN,memp_memory );
//	myfree ( SRAMIN,ram_heap );

	myfree ( SRAMCCM,memp_memory );
	myfree ( SRAMCCM,ram_heap );	
}
//lwip 默认IP设置
//lwipx:lwip控制结构体指针
void lwip_comm_default_ip_set ( __lwip_dev* lwipx )
{
//    //02:00:00:52:48:50
//	uint32_t sn0 = *(__I uint32_t *)(0x1FFF7A10+0x08); //动态MAC

	//默认远端IP为:192.168.1.100
	lwipx->remoteip[0]=192;
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=110;
	lwipx->remoteip[3]=79;
	

	calcMac ( lwipx->mac );

	//默认本地IP为:192.168.1.30
//	lwipx->ip[0]=192;
//	lwipx->ip[1]=168;
//	lwipx->ip[2]=0;
//	lwipx->ip[3]=155;

	memcpy(lwipx->ip,gDevBaseParam.localIP.ip,4);
	
	//默认子网掩码:255.255.255.0
//	lwipx->netmask[0]=255;
//	lwipx->netmask[1]=255;
//	lwipx->netmask[2]=255;
//	lwipx->netmask[3]=0;
	memcpy(lwipx->netmask,gDevBaseParam.localIP.netMask,4);
	
	//默认网关:192.168.1.1
//	lwipx->gateway[0]=192;
//	lwipx->gateway[1]=168;
//	lwipx->gateway[2]=0;
//	lwipx->gateway[3]=1;
	memcpy(lwipx->gateway,gDevBaseParam.localIP.gateWay,4);
	
	lwipx->dhcpstatus=0;//没有DHCP
}

//LWIP初始化(LWIP启动的时候使用)
//返回值:0,成功
//      1,内存错误
//      2,LAN8720初始化失败
//      3,网卡添加失败.
u8 lwip_comm_init ( void )
{
	struct netif* Netif_Init_Flag;		//调用netif_add()函数时的返回值,用于判断网络初始化是否成功
	struct ip_addr ipaddr;  			//ip地址
	struct ip_addr netmask; 			//子网掩码
	struct ip_addr gw;      			//默认网关

	if ( ETH_Mem_Malloc() )
	{	
        log_d ( "ETH_Mem_Malloc error\r\n" );
		return 1;    //内存申请失败
	}
	log_d ( "ETH_Mem_Malloc\r\n" );

	if ( lwip_comm_mem_malloc() )
	{
	    log_d ( "lwip_comm_mem_malloc error\r\n" );
		return 2;    //内存申请失败
	}
	log_d ( "lwip_comm_mem_malloc\r\n" );

	lwip_comm_default_ip_set ( &lwipdev );	//设置默认IP等信息
	log_d ( "lwip_comm_default_ip_set\r\n" );

    LAN8720_Init();
    
	tcpip_init ( NULL,NULL );				//初始化tcp ip内核,该函数里面会创建tcpip_thread内核任务
	log_d ( "tcpip_init\r\n" );

    log_d("lwip_comm_init gDevBaseParam.localIP.ipMode.iMode = %x\r\n",gDevBaseParam.localIP.ipMode.iMode);

    if(gDevBaseParam.localIP.ipMode.iMode == DHCP_IP)
    {
        ipaddr.addr = 0;
        netmask.addr = 0;
        gw.addr = 0;
        log_d("set get ip mode is dhcp\r\n");
    }
    else
    {
    	IP4_ADDR ( &ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3] );
    	IP4_ADDR ( &netmask,lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3] );
    	IP4_ADDR ( &gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3] );
    	
    	log_d ( "网卡的MAC地址为:................%02x.%02x.%02x.%02x.%02x.%02x\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5] );
    	log_d ( "静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3] );
    	log_d ( "子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3] );
    	log_d ( "默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3] );
    	
    	
    }

	Netif_Init_Flag=netif_add ( &lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input ); //向网卡列表中添加一个网口
	if ( Netif_Init_Flag==NULL )
	{
	    log_d("netif_add error\r\n");
		return 4;    //网卡添加失败
	}
	else//网口添加成功后,设置netif为默认值,并且打开netif网口
	{
		netif_set_default ( &lwip_netif ); //设置netif为默认网口	
		log_d("netif_set_default\r\n");
	}

	if(netif_is_link_up(&lwip_netif))
	{
		netif_set_up(&lwip_netif); //打开netif网口
	    log_d("netif_set_up\r\n");
		
		if(gDevBaseParam.localIP.ipMode.iMode == DHCP_IP)
		{
			SetGB_DHCPState(DHCP_START);			
	        log_d("SetGB_DHCPState(DHCP_START)\r\n");
		}	
	}
	else
	{
		/*  When the netif link is down this function must be called.*/
		netif_set_down(&lwip_netif);
		log_d("netif_set_down\r\n");
		if(gDevBaseParam.localIP.ipMode.iMode == DHCP_IP)
		{
			SetGB_DHCPState(DHCP_LINK_DOWN);
			log_d("SetGB_DHCPState(DHCP_LINK_DOWN)\r\n");
		}	
	}      

//	if(Link_Down == GetGB_LinkState())
//	{
//		netif_set_up(&lwip_netif); //打开netif网口
//		
//		#if LWIP_DHCP
//	    SetGB_DHCPState(DHCP_START);
//		#endif
//	}
//	else
//	{
//		/*  When the netif link is down this function must be called.*/
//		netif_set_down(&lwip_netif);
//		
//		#if LWIP_DHCP
//		SetGB_DHCPState(DHCP_LINK_DOWN);
//		#endif
//	}    

	/* Set the link callback function, this function is called on change of link status*/
	netif_set_link_callback(&lwip_netif, ETH_link_callback);

    log_d("lwip_comm_init finish\r\n");
    
	return 0;//操作OK.
}





/***************************************************************************************************
*FunctionName：SetGB_LinkState
*Description：更新网线连接状态
*Input：None
*Output：None
*Author：xsx
*Data：2016年3月9日15:16:33
***************************************************************************************************/
void SetGB_LinkState(LinkState_TypeDef linkstate)
{
	GB_LinkState = linkstate;
}
/***************************************************************************************************
*FunctionName：GetGB_LinkState
*Description：获取网线连接状态
*Input：None
*Output：None
*Author：xsx
*Data：2016年3月9日15:16:48
***************************************************************************************************/
LinkState_TypeDef GetGB_LinkState(void)
{
	return GB_LinkState;
}

/***************************************************************************************************
*FunctionName：ETH_link_callback
*Description：网线连接状态改变回调函数
*Input：None
*Output：None
*Author：xsx
*Data：2016年3月9日17:02:01
***************************************************************************************************/
void ETH_link_callback(struct netif *netif)
{

	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;

	if(netif_is_link_up(netif))
	{
		printf("<<<<<<<<<<<<<<<<link up>>>>>>>>>>>>>>>>> \r\n");
		/* Restart MAC interface */
		ETH_Start();

		if(gDevBaseParam.localIP.ipMode.iMode == DHCP_IP)
		{
			ipaddr.addr = 0;
			netmask.addr = 0;
			gw.addr = 0;

			SetGB_DHCPState(DHCP_START);
	    }
		else
		{
            IP4_ADDR ( &ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3] );
            IP4_ADDR ( &netmask,lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3] );
            IP4_ADDR ( &gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3] );
            log_d( "cb 网卡的MAC = %02x.%02x.%02x.%02x.%02x.%02x\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5] );
            log_d ( "cb ip = %d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3] );
            log_d ( "cb netmask = %d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3] );
            log_d ( "cb gateway = %d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3] );
		}

		netif_set_addr(&lwip_netif, &ipaddr , &netmask, &gw);
    
		/* When the netif is fully configured this function must be called.*/
		netif_set_up(&lwip_netif);    
	}
	else
	{
		printf("<<<<<<<<<<<<<<<<link down>>>>>>>>>>>>>>>>> \r\n");
		gConnectStatus = 0;//把联接标志置为失去联接
		ETH_Stop();
		
		if(gDevBaseParam.localIP.ipMode.iMode == DHCP_IP)
		{
			SetGB_DHCPState(DHCP_LINK_DOWN);
			dhcp_stop(netif);
		}
	
		

		/*  When the netif link is down this function must be called.*/
		netif_set_down(&lwip_netif);
	}
}


void StartEthernet(void)
{
	//* 初始化LwIP
	lwip_comm_init();

	printf("gDevBaseParam.localIP.ipMode.iMode = %x\r\n",gDevBaseParam.localIP.ipMode.iMode);
	
    if(gDevBaseParam.localIP.ipMode.iMode == DHCP_IP)
    {
        
        StartvLwipDHCPTask(&lwip_netif);
    }

	
	StartvLwipComTask(&lwip_netif);
}





