<template>
  <div class="new-page" :style="`min-height: ${pageMinHeight}px`">
    <img v-if="ledStatus" alt="led" class="led" src="@/assets/img/led_on.png" @click="onSwitchChange(false)" />
    <img v-else alt="led" class="led" src="@/assets/img/led_off.png" @click="onSwitchChange(true)" />
    <!--    <a-switch :checked="ledStatus" checked-children="开" un-checked-children="关" :loading="loading" @change="onSwitchChange"></a-switch>-->
    <!--    <oui-form uci-config="system">-->
    <!--      <oui-typed-section v-if="sysfs.length > 0" v-slot="{ s }" :title="$t('LED Configuration')" type="led" addremove :teasers="['name', 'sysfs', 'default', 'trigger']">-->
    <!--        <oui-form-item-input :uci-section="s" :label="$t('leds.Name')" name="name" required></oui-form-item-input>-->
    <!--        <oui-form-item-select :uci-section="s" :label="$t('leds.Led Name')" name="sysfs" :options="sysfs" required></oui-form-item-select>-->
    <!--        <oui-form-item-switch :uci-section="s" :label="$t('leds.Default state')" name="default"></oui-form-item-switch>-->
    <!--        <oui-form-item-select :uci-section="s" :label="$t('leds.Trigger')" name="trigger" :options="triggers" initial="none" required></oui-form-item-select>-->
    <!--        <oui-form-item-input :uci-section="s" :label="$t('leds.On-State Delay')" name="delayon" required rules="uinteger" depend="trigger == 'timer'" :help="$t('leds.Time in milliseconds the LED stays on')"></oui-form-item-input>-->
    <!--        <oui-form-item-input :uci-section="s" :label="$t('leds.Off-State Delay')" name="delayoff" required rules="uinteger" depend="trigger == 'timer'" :help="$t('leds.Time in milliseconds the LED stays off')"></oui-form-item-input>-->
    <!--        <oui-form-item-select :uci-section="s" :label="$t('leds.Device')" name="dev" :options="devices" depend="trigger == 'netdev'" required allow-create></oui-form-item-select>-->
    <!--        <oui-form-item-select :uci-section="s" :label="$t('leds.Trigger Mode')" name="mode" :options="modes" multiple depend="trigger == 'netdev'"></oui-form-item-select>-->
    <!--      </oui-typed-section>-->
    <!--      <a-alert v-else>{{ $t('leds.No LEDs available') }}</a-alert>-->
    <!--    </oui-form>-->
  </div>
</template>

<script>
import { mapState } from 'vuex';
import i18n from './i18n';
import { homeService, sysConfService } from '@/services';
import { SysInfoType } from '@/services/home';
import { SysConfType } from '@/services/sysconf';
export default {
  name: 'LEDs',
  i18n: i18n,
  data() {
    return {
      sysfs: [],
      triggers: [],
      devices: [],
      modes: [
        ['link', this.$t('leds.Link On')],
        ['tx', this.$t('leds.Transmit')],
        ['rx', this.$t('leds.Receive')]
      ],
      loading: false,
      ledStatus: false,
      interval: null,
      requestStatus: false
    };
  },
  computed: {
    ...mapState('setting', ['pageMinHeight'])
  },
  async created() {
    this.getLedInfo();
    sysConfService.sysConf(SysConfType.GET).then(({ data }) => {
      this.interval = setInterval(() => {
        this.getLedInfo();
      }, Number(data.split('&')[1]));
    }).catch(e => {
      console.error(e);
    });
    // 此处请求数据
    // this.listLEDs().then(({ leds }) => {
    //   if (leds.length === 0) { return }
    //
    //   leds.forEach(led => {
    //     this.sysfs.push(led.name)
    //   })
    //
    //   leds[0].triggers.forEach(trigger => {
    //     this.triggers.push(trigger)
    //   })
    // })
    //
    // this.$network.load().then(() => {
    //   this.devices = this.$network.getDevices().map(dev => dev.name).filter(name => name !== 'lo')
    // })
  },
  beforeDestroy() {
    clearInterval(this.interval);
  },
  methods: {
    listLEDs() {
      // return this.$rpc.call('system', 'led_list')
    },
    // 获取led信息
    getLedInfo() {
      if (this.requestStatus) return;
      this.requestStatus = true;
      homeService.getSysInfo(SysInfoType.LED_STATUS).then(({ data }) => {
        this.ledStatus = data.toLowerCase() === 'on';
      }).catch(e => {
        console.error(e);
      }).finally(() => {
        this.requestStatus = false;
        console.log('led请求已完成');
      });
    },
    onSwitchChange(e) {
      // console.log(e);
      this.loading = true;
      homeService.getSysInfo(e ? SysInfoType.ON : SysInfoType.OFF).then(({ data }) => {
        // console.log(data);
        if (data === 'on') {
          this.ledStatus = true;
          this.$message.success('led已经打开');
        } else {
          this.ledStatus = false;
          this.$message.success('led已经关闭');
        }
      }).catch(e => {
        console.error(e);
      }).finally(_ => {
        this.loading = false;
      });
    }
  }
};
</script>

<style scoped lang="less">
@import "index";
</style>
