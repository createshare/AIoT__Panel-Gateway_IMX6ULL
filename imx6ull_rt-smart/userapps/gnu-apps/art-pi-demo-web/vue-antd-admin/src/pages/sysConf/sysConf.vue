<template>
  <div class="new-page" :style="`min-height: ${pageMinHeight}px`">
    <a-table bordered :data-source="dataSource" :columns="columns" :loading="loading" size="middle" row-key="configName" :pagination="false">
      <template slot="configVal" slot-scope="text, record">
        <editable-cell :text="text" @change="onCellChange(record.code, $event)"></editable-cell>
      </template>
    </a-table>
  </div>
</template>

<script>
import { mapState } from 'vuex';
import i18n from './i18n';
import { sysConfService } from '@/services';
import { SysConfType } from '@/services/sysconf';
import EditableCell from './EditableCell';
export default {
  name: 'SysConf',
  i18n: i18n,
  components: {
    EditableCell
  },
  data() {
    return {
      columns: [{ title: '配置项', dataIndex: 'configName', width: '30%' },
        { title: '值', dataIndex: 'configVal', scopedSlots: { customRender: 'configVal' } }],
      dataSource: [],
      loading: false
    };
  },
  computed: {
    ...mapState('setting', ['pageMinHeight'])
  },
  created() {
    this.getSysConf();
  },
  methods: {
    getSysConf() {
      this.loading = true;
      sysConfService.sysConf(SysConfType.GET).then(({ data }) => {
        console.log(data);
        data = data.split('&');
        this.dataSource = [{ configName: '系统信息刷新间隔(毫秒)', configVal: Number(data[0]), code: 'SET_SYS_INFO_INTERVAL' },
          { configName: 'LED 状态刷新间隔(毫秒)', configVal: Number(data[1]), code: 'SET_LED_INFO_INTERVAL' },
          { configName: '系统时间同步间隔(秒)', configVal: Number(data[2]), code: 'SET_SYS_TIME_INTERVAL' }];
      }).catch(e => {
        console.error(e);
      }).finally(() => {
        this.loading = false;
      });
    },
    setSysConf(key, value) {
      sysConfService.sysConf(SysConfType[key], value).then(({ data }) => {
        console.log(data);
        this.getSysConf();
      }).catch(e => {
        console.error(e);
      }).finally(() => {
        this.loading = false;
      });
    },
    onCellChange(key, value) {
      this.setSysConf(key, value);
    }
  }
};
</script>

<style scoped lang="less">
@import "index";
</style>
