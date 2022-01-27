<template>
  <div class="new-page" :style="`min-height: ${pageMinHeight}px`">
    <div style="position: relative; top: 70px">
      <oui-line :length="lineLength" :style="{ position: 'absolute', left: lineOffset }" active></oui-line>
      <oui-line :length="lineLength" :style="{ position: 'absolute', right: lineOffset }" active :disconnect="!wanIsUp"></oui-line>
    </div>
    <div class="oui-home-tabs">
      <div>
        <div style="display:inline;" class="tab-title" @click="tab = 'router'">
          <img src="/icons/devices.png" />
          <!--          <div :class="{active: tab === 'devices'}">{{ $t('Terminal devices') + `(${devices.length})` }}</div>-->
        </div>
        <div style="display:inline;" class="tab-title" @click="tab = 'router'">
          <img src="/icons/router.png" />
          <!--          <div :class="{active: tab === 'router'}">{{ $t('System') }}</div>-->
        </div>
        <div style="display:inline;" class="tab-title" @click="tab = 'router'">
          <img src="/icons/internet.png" />
          <!--          <div :class="{active: tab === 'internet'}">WAN</div>-->
        </div>
      </div>
      <div style="clear: both">
        <!--        <div v-show="tab === 'devices'">-->
        <!--          <a-card :title="$t('Online devices')" style="margin-bottom: 20px">-->
        <!--            <a-table :columns="devicesColumns" :data-source="devices"></a-table>-->
        <!--          </a-card>-->
        <!--          <a-card :title="$t('Active DHCP Leases')" style="margin-bottom: 20px">-->
        <!--            <a-table :columns="leasesColumns" :data-source="leases"></a-table>-->
        <!--          </a-card>-->
        <!--          <a-card :title="$t('Active DHCPv6 Leases')" style="margin-bottom: 20px">-->
        <!--            <a-table :columns="leases6Columns" :data-source="leases6"></a-table>-->
        <!--          </a-card>-->
        <!--          <a-card :title="$t('Associated Stations')" style="margin-bottom: 20px">-->
        <!--            <a-table :columns="assoclistColumns" :data-source="assoclist">-->
        <!--              <template #signal="record">-->
        <!--                <img :src="wifiSignalIcon(record)" />-->
        <!--                <span>{{ ' ' + record.signal + '/' + record.noise + ' dBm' }}</span>-->
        <!--              </template>-->
        <!--              <template #rx="record">-->
        <!--                <span>{{ formatWifiRxRate(record) }}</span>-->
        <!--              </template>-->
        <!--              <template #tx="record">-->
        <!--                <span>{{ formatWifiTxRate(record) }}</span>-->
        <!--              </template>-->
        <!--            </a-table>-->
        <!--          </a-card>-->
        <!--        </div>-->
        <div v-show="tab === 'router'">
          <div style="display: flex; justify-content: space-around;">
            <oui-dashboard :percent="cpuPercentage" :label="$t('CPU usage')"></oui-dashboard>
            <oui-dashboard :percent="memPercentage" :label="$t('Memory usage')">
              <template #desc>{{ memMessage }}</template>
            </oui-dashboard>
            <oui-dashboard :percent="storagePercentage" :label="$t('Storage')">
              <template #desc>{{ diskMessage }}</template>
            </oui-dashboard>
          </div>
          <a-descriptions :title="$t('System information')" :column="1" bordered style="text-align: left;margin-top: 20px;">
            <a-descriptions-item v-for="item in sysinfo" :key="item[0]" :label="item[0]">{{ item[1] }}</a-descriptions-item>
          </a-descriptions>
        </div>
        <!--        <div v-show="tab === 'internet'">-->
        <!--          <a-descriptions :title="$t('Network')" :column="1" bordered>-->
        <!--            <a-descriptions-item v-for="item in waninfo" :key="item[0]" :label="item[0]">{{ item[1] }}</a-descriptions-item>-->
        <!--          </a-descriptions>-->
        <!--        </div>-->
      </div>
    </div>
  </div>
</template>

<script>
import { mapState } from 'vuex';
import OuiLine from '@/components/OuiLine/OuiLine';
import OuiDashboard from '@/components/OuiDashboard/OuiDashboard';
import i18n from './i18n';
import { homeService, sysConfService } from '@/services';
import { SysInfoType } from '@/services/home';
import { SysConfType } from '@/services/sysconf';
export default {
  name: 'Home',
  components: { OuiDashboard, OuiLine },
  i18n: i18n,
  data() {
    return {
      interval: null,
      tab: 'router',
      statusLineLength: '26%',
      sysinfo: [],
      waninfo: [],
      wanIsUp: false,
      devicesColumns: [
        { dataIndex: 'hostname', title: this.$t('Hostname') },
        { dataIndex: 'ipaddr', title: this.$t('IPv4-Address') },
        { dataIndex: 'macaddr', title: this.$t('MAC-Address') },
        { dataIndex: 'rxrate', title: this.$t('RX Rate') },
        { dataIndex: 'txrate', title: this.$t('TX Rate') }
      ],
      devices: [],
      devicesMap: {},
      leasesColumns: [
        { dataIndex: 'hostname', title: this.$t('Hostname') },
        { dataIndex: 'ipaddr', title: this.$t('IPv4-Address') },
        { dataIndex: 'macaddr', title: this.$t('MAC-Address') },
        { dataIndex: 'leasetime', title: this.$t('Leasetime remaining'), customRender: text => this.leasetime(text) }
      ],
      leases: [],
      leases6Columns: [
        { dataIndex: 'name', title: this.$t('Host') },
        { dataIndex: 'ip6addr', title: this.$t('IPv6-Address') },
        { dataIndex: 'duid', title: 'DUID' },
        { dataIndex: 'leasetime', title: this.$t('Leasetime remaining'), customRender: text => this.leasetime(text) }
      ],
      leases6: [],
      assoclistColumns: [
        { dataIndex: 'mac', title: this.$t('MAC-Address') },
        { dataIndex: 'host', title: this.$t('Host') },
        { key: 'signal', title: this.$t('Signal') + ' / ' + this.$t('Noise'), scopedSlots: { customRender: 'signal' } },
        { key: 'rx', title: this.$t('RX Rate'), scopedSlots: { customRender: 'rx' } },
        { key: 'tx', title: this.$t('TX Rate'), scopedSlots: { customRender: 'tx' } }
      ],
      assoclist: [],
      lastCPUTime: null,
      cpuPercentage: 0,
      memPercentage: 0,
      storagePercentage: 0,
      memMessage: '',
      diskMessage: '',
      requestStatus: false
    };
  },
  timers: {
    update: { time: 2000, autostart: true, immediate: true, repeat: true },
    updateCpuUsage: { time: 1000, autostart: true, immediate: true, repeat: true }
  },
  async created() {
    this.getSysDesc();
    this.getSysInfo();
    sysConfService.sysConf(SysConfType.GET).then(({ data }) => {
      this.interval = setInterval(() => {
        this.getSysInfo();
      }, Number(data.split('&')[0]));
    }).catch(e => {
      console.error(e);
    });
  },
  beforeDestroy() {
    clearInterval(this.interval);
  },
  methods: {
    // 获取内存cpu硬盘信息
    getSysInfo() {
      if (this.requestStatus) return;
      this.requestStatus = true;
      homeService.getSysInfo(SysInfoType.STATUS).then(({ data }) => {
        // console.log(data);
        //  3.0MB&126.5MB;35.0MB&7.4GB;0.19;up  （内存及剩余空间）
        const [mem, disk, cpu, up] = data.split(';');
        // console.log(mem, disk);
        const [usedMem, unusedMem] = mem.split('&');
        const [usedDisk, unusedDisk] = disk.split('&');
        const UNIT = {
          MB: 'MB',
          GB: 'GB'
        };
        const sysInfoArr = [0, 0, 0, 0];
        [usedMem, unusedMem, usedDisk, unusedDisk].forEach((item, idx) => {
          if (item.indexOf(UNIT.GB) > -1) {
            sysInfoArr[idx] = parseFloat(item.substr(0, item.length - 2)) * 1024;
          } else if (item.indexOf(UNIT.MB) > -1) {
            sysInfoArr[idx] = parseFloat(item.substr(0, item.length - 2));
          }
        });
        // console.log(sysInfoArr);
        this.memPercentage = Math.ceil(sysInfoArr[0] * 100 / sysInfoArr[1]);
        this.storagePercentage = Math.ceil(sysInfoArr[2] * 100 / sysInfoArr[3]);
        this.memMessage = `${usedMem}/${unusedMem}`;
        this.diskMessage = `${usedDisk}/${unusedDisk}`;
        this.cpuPercentage = Number(cpu);
        this.wanIsUp = up === 'up';
      }).catch(e => {
        console.error(e);
      }).finally(() => {
        this.requestStatus = false;
        // console.log('led请求已完成');
      });
    },
    // 获取系统简介
    getSysDesc() {
      homeService.getSysInfo(SysInfoType.INFO).then(({ data }) => {
        // console.log(data);
        /**
         * 系统版本   Thread Smart Operating System 5.0.0
         * 编译日期   Aug  9 2021
         * GCC版本    7.3.0
         * 网络接口 e1
         * MAC 地址 a8 5e 45 91 92 93
         * IP  地址 192.168.111.139
         * 网关地址 192.168.111.1
         * 子网掩码 255.255.255.0
         * DNS 地址 114.114.115.115
         */
        const val = data.split('&');
        const labels = ['系统版本', '编译日期', 'GCC版本', '网络接口', 'MAC地址', 'IP  地址', '网关地址', '子网掩码', 'DNS地址'];
        this.sysinfo = labels.map((item, index) => {
          return [item, val[index]];
        });
      }).catch(e => {
        console.error(e);
      });
    },
    leasetime(expires) {
      return expires <= 0 ? this.$t('expired') : '%t'.format(expires);
    },
    wifirate(sta, t) {
      const rate = sta[t + '_rate'];
      const bw = sta[t + '_mhz'];
      let s = '%.1f Mbit/s'.format(rate / 1000);
      s += `, ${bw}MHz`;

      return s;
    },
    wifiSignalIcon(s) {
      let q = (-1 * (s.noise - s.signal)) / 5;
      if (q < 1) { q = 1; } else if (q < 2) { q = 2; } else if (q < 3) { q = 3; } else if (q < 4) { q = 4; } else { q = 5; }
      return `/icons/signal_${q}.png`;
    },
    formatWifiRxRate(row) {
      return this.wifirate(row, 'rx');
    },
    formatWifiTxRate(row) {
      return this.wifirate(row, 'tx');
    },
    updateCpuUsage() {
      this.$rpc.call('system', 'cpu_time').then(times => {
        if (!this.lastCPUTime) {
          this.cpuPercentage = 0;
          this.lastCPUTime = times;
          return;
        }

        const idle1 = this.lastCPUTime[3];
        const idle2 = times[3];

        let total1 = 0;
        let total2 = 0;

        this.lastCPUTime.forEach(t => {
          total1 += t;
        });

        times.forEach(t => {
          total2 += t;
        });

        this.cpuPercentage = Math.floor(((total2 - total1 - (idle2 - idle1)) / (total2 - total1)) * 100);
        this.lastCPUTime = times;
      });
    },
    update() {
      this.$system.getInfo().then(({ hostname, model, system, release, kernel, localtime, uptime, memory }) => {
        this.sysinfo = [
          [this.$t('Hostname'), hostname],
          [this.$t('Model'), model],
          [this.$t('Architecture'), system],
          [this.$t('Firmware Version'), release.revision],
          [this.$t('Kernel Version'), kernel],
          [this.$t('Local Time'), new Date(localtime * 1000).toString()],
          [this.$t('Uptime'), '%t'.format(uptime)]
        ];

        this.memPercentage = Math.floor(((memory.total - memory.free) / memory.total) * 100);
      });

      this.$system.getDiskInfo().then(({ root }) => {
        this.storagePercentage = Math.floor((root.used / root.total) * 100);
      });

      this.$network.load().then(() => {
        const iface = this.$network.getInterface('wan');
        if (!iface) {
          this.waninfo = [];
          this.wanIsUp = false;
          return;
        }

        this.waninfo = [
          [this.$t('IP Address'), iface.getIPv4Addrs().join(', ')],
          [this.$t('Gateway'), iface.getIPv4Gateway()],
          ['DNS', iface.getDNSAddrs().join(', ')]
        ];
        this.wanIsUp = iface.isUp();
      });

      this.$rpc.call('network', 'dhcp_leases').then(r => {
        const leasesMap = {};

        this.leases = r.leases.map(l => {
          l.key = l.ipaddr;
          return l;
        });

        this.leases.forEach(l => {
          leasesMap[l.macaddr] = { hostname: l.hostname, ipaddr: l.ipaddr };
        });

        this.$rpc.call('network', 'bwm').then(r => {
          this.devices = r.entries.map(dev => {
            const lease = leasesMap[dev.macaddr];

            dev = { ...dev, txrate: 0, rxrate: 0 };

            const ldev = this.devicesMap[dev.macaddr];
            if (ldev) {
              dev.txrate = '%mB/s'.format((dev.tx - ldev.tx) / 2);
              dev.rxrate = '%mB/s'.format((dev.rx - ldev.rx) / 2);
            }

            this.devicesMap[dev.macaddr] = { tx: dev.tx, rx: dev.rx };

            if (lease) dev.hostname = lease.hostname;

            dev.key = dev.macaddr;

            return dev;
          });
        });

        this.$wireless.getAssoclist().then(assoclist => {
          this.assoclist = assoclist.map(sta => {
            const lease = leasesMap[sta.mac.toLowerCase()];
            if (lease) sta.host = `${lease.hostname} (${lease.ipaddr})`;

            sta.key = sta.mac;

            return sta;
          });
        });
      });

      this.$rpc.call('network', 'dhcp6_leases').then(r => {
        this.leases6 = r.leases.map(l => {
          l.key = l.ipaddr;
          return l;
        });
      });
    }
  },
  computed: {
    ...mapState('setting', ['pageMinHeight']),
    lineLength() {
      return 'calc(100% / 3 - 160px)';
    },
    lineOffset() {
      return 'calc(100% / 6 + 66px)';
    }
  }
};
</script>

<style scoped lang="less">
@import "index";
</style>
