<template>
  <div class="new-page" :style="`min-height: ${pageMinHeight}px`">
    <a-table bordered :columns="columns" :data-source="dataSource" :loading="loading" size="middle" row-key="index">
      <template #msgTitle>
        <a-input v-model="search" :addon-before="$t('syslog.Message')" :placeholder="$t('syslog.Enter keywords to filter')" allow-clear></a-input>
      </template>
      <template #facility="scope">
        {{ $t(`syslog.facility.${scope}`) }}
      </template>
      <template #level="scope">
        {{ $t(`syslog.level.${scope}`) }}
      </template>
      <template #operate="scope">
        {{ $t(`syslog.operate.${scope}`) }}
      </template>
      <template slot="operation" slot-scope="text, record">
        <a-popconfirm
          v-if="dataSource.length"
          title="确定删除该日志?"
          @confirm="() => deleteLog(record.datetime)"
        >
          <a href="javascript:;"><a-icon type="delete"></a-icon>删除</a>
        </a-popconfirm>
      </template>
    </a-table>
  </div>
</template>

<script>
import { mapState } from 'vuex';
import i18n from './i18n';
import { sysLogService } from '@/services';
export default {
  name: 'SysLog',
  i18n: i18n,
  data() {
    return {
      columns: [
        { dataIndex: 'index', title: this.$t('syslog.Index'), width: 60, align: 'center' },
        { dataIndex: 'datetime', title: this.$t('syslog.Datetime'), width: 220, align: 'center' },
        { dataIndex: 'facility', title: this.$t('syslog.Facility'), scopedSlots: { customRender: 'facility' }, align: 'center' },
        { dataIndex: 'level', title: this.$t('syslog.Level'), scopedSlots: { customRender: 'level' }, align: 'center' },
        { dataIndex: 'operate', title: this.$t('syslog.Operate'), scopedSlots: { customRender: 'operate' }, align: 'center' },
        { dataIndex: 'detail', title: this.$t('syslog.Detail'), align: 'center' },
        { dataIndex: 'operation', title: this.$t('syslog.Operate'), scopedSlots: { customRender: 'operation' }, align: 'center' }
      ],
      loading: false,
      syslog: [],
      search: ''
    };
  },
  computed: {
    ...mapState('setting', ['pageMinHeight']),
    dataSource() {
      return this.syslog.filter(log => !this.search || log.msg.includes(this.search));
    }
  },
  created() {
    this.loading = false;
    this.getLogList();
    // this.$rpc.call('system', 'syslog').then(({ log }) => {
    //   this.syslog = log.map((v, i) => {
    //     v.key = i
    //     return v
    //   })
    //   this.loading = false
    // })
  },
  beforeDestroy() {
    console.log('destroy');
  },
  methods: {
    getLogList() {
      this.loading = true;
      sysLogService.getSysLogList().then(res => {
        console.log(res);
        // 因后台限制需要去除最后一个'&'符号
        const logs = res.data.replaceAll('\u0000', ':').substr(0, res.data.length - 1).split('&');
        this.syslog = logs.map((log, index) => {
          const logArr = log.split(';');
          return {
            index: index + 1,
            datetime: logArr[0],
            facility: logArr[1], // 类型
            level: logArr[2], // 级别
            operate: logArr[3], // 操作
            detail: logArr[4]
          };
        });
      }).catch(e => {
        console.error(e);
      }).finally(() => {
        this.loading = false;
      });
    },
    deleteLog(time) {
      sysLogService.delSysLog(time).then(res => {
        // this.getLogList()
      }).finally(() => {
        this.getLogList();
      });
    }
  }
};
</script>

<style scoped lang="less">
@import "index";
</style>
