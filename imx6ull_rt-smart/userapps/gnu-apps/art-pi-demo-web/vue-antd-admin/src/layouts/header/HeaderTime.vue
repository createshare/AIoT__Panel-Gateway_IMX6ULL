<template>
  <span>{{ format(new Date(time),'yyyy-MM-dd HH:mm:ss') }}</span>
</template>

<script>
import { homeService, sysConfService } from '@/services';
import { SysConfType } from '@/services/sysconf';
import { format } from 'date-fns';
export default {
  name: 'HeaderTime',
  data() {
    return {
      format,
      time: 0,
      sInterval: null,
      interval: null
    };
  },
  created() {
    this.getSysTime();
    sysConfService.sysConf(SysConfType.GET).then(({ data }) => {
      this.interval = setInterval(() => {
        this.getSysTime();
      }, Number(data.split('&')[2]) * 1000);
    }).catch(e => {
      console.error(e);
    });
  },
  beforeDestroy() {
    clearInterval(this.interval);
  },
  methods: {
    getSysTime() {
      clearInterval(this.sInterval);
      homeService.getSysTime().then(({ data }) => {
        this.time = new Date(data.replaceAll('\u0000', ':')).getTime();
        this.sInterval = setInterval(() => {
          this.time += 1000;
        }, 1000);
      }).catch(e => {
        console.error(e);
      });
    }
  }
};
</script>

<style scoped>

</style>
