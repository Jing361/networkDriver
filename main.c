#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>

struct os_packet
{
   struct net_device *dev;
   int datalen;
   u8 data[ETH_DATA_LEN];
};

struct os_priv
{
   struct net_device_stats stats;
   int status;
   struct os_packet *pkt;
   int rx_int_enabled;
   int tx_packetlen;
   u8 *tx_packetdata;
   struct sk_buff *skb;
   spinlock_t lock;
   struct net_device *dev;
};

struct net_device *os0;
struct net_device *os1;

struct os_priv *priv0;
struct os_priv *priv1;

int os_open(struct net_device *dev)
{
   netif_start_queue(dev);
   return 0;
}

int os_stop(struct net_device *dev)
{
   netif_start_queue(dev);
   return 0;
}

int os_start_xmit(struct sk_buff *skb, struct net_device *dev){ return 0; }

struct net_device_stats *os_stats(struct net_device *dev)
{
   return &(((struct os_priv*)netdev_priv(dev))->stats);
}

static const struct net_device_ops os_device_ops =
{
   .ndo_open = os_open,
   .ndo_stop = os_stop,
   .ndo_start_xmit = os_start_xmit,
   .ndo_get_stats = os_stats,
};

int os_header(struct sk_buff *skb, struct net_device *dev,
              unsigned short type, const void *daddr,
              const void *saddr, unsigned int len)
{
   return 0;
}

static const struct header_ops os_header_ops = 
{
   .create = os_header,
};

int tester1_init (void)
{
   int i;

   os0 = alloc_etherdev(sizeof(struct os_priv));
   os1 = alloc_etherdev(sizeof(struct os_priv));

   priv0 = kmalloc(sizeof(struct os_priv), GFP_KERNEL);
   priv1 = kmalloc(sizeof(struct os_priv), GFP_KERNEL);

   for(i = 0; i < 6; i++)
   {
      os0->dev_addr[i] = (unsigned char)i;
      os1->dev_addr[i] = (unsigned char)i;

      os0->broadcast[i] = (unsigned char)15;
      os1->broadcast[i] = (unsigned char)15;
   }

   // change the mac address of os1 to be different
   os1->dev_addr[5]++;

   os0->hard_header_len = 14;
   os1->hard_header_len = 14;

   memcpy(os0->name, "os0\0", 4);
   memcpy(os1->name, "os1\0", 4);

   os0->netdev_ops = &os_device_ops;
   os1->netdev_ops = &os_device_ops;

   os0->header_ops = &os_header_ops;
   os1->header_ops = &os_header_ops;

   os0->flags |= IFF_NOARP;
   //os1->flags |= IFF_NOARP;

   memset(priv0, 0, sizeof(struct os_priv));
   memset(priv1, 0, sizeof(struct os_priv));

   priv0->dev = os0;
   priv1->dev = os1;

   spin_lock_init(&priv0->lock);
   spin_lock_init(&priv1->lock);

   priv0->rx_int_enabled = 1;
   priv1->rx_int_enabled = 1;

   priv0->pkt = kmalloc(sizeof(struct os_packet), GFP_KERNEL);
   priv1->pkt = kmalloc(sizeof(struct os_packet), GFP_KERNEL);

   priv0->pkt->dev = os0;
   priv1->pkt->dev = os1;

   register_netdev(os0);
   register_netdev(os1);

   printk(KERN_INFO "Module loaded\n");

   return 0;
}

void tester1_cleanup(void)
{
   struct os_priv *priv;
   if(os0)
   {
      priv = netdev_priv(os0);
      kfree(priv->pkt);
      unregister_netdev(os0);
      kfree(os0);
      kfree(priv0);
   }

   if(os1)
   {
      priv = netdev_priv(os1);
      kfree(priv->pkt);
      unregister_netdev(os1);
      kfree(os1);
      kfree(priv1);
   }
   
   printk(KERN_INFO "Module unloaded\n");
}

module_init(tester1_init);
module_exit(tester1_cleanup);
