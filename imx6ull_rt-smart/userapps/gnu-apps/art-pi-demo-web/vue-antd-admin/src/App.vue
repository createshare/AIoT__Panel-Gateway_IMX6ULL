<template>
  <a-config-provider :locale="locale" :get-popup-container="popContainer">
    <router-view></router-view>
  </a-config-provider>
</template>

<script>
import { enquireScreen } from './utils/util';
import { mapState, mapMutations } from 'vuex';
import themeUtil from '@/utils/themeUtil';
import { getI18nKey } from '@/utils/routerUtil';
export default {
  name: 'App',
  data() {
    return {
      locale: {}
    };
  },
  watch: {
    weekMode(val) {
      this.setWeekModeTheme(val);
    },
    lang(val) {
      this.setLanguage(val);
      this.setHtmlTitle();
    },
    $route() {
      this.setHtmlTitle();
    },
    'theme.mode': function(val) {
      const closeMessage = this.$message.loading(`您选择了主题模式 ${val}, 正在切换...`);
      themeUtil.changeThemeColor(this.theme.color, val).then(closeMessage);
    },
    'theme.color': function(val) {
      const closeMessage = this.$message.loading(`您选择了主题色 ${val}, 正在切换...`);
      themeUtil.changeThemeColor(val, this.theme.mode).then(closeMessage);
    },
    'layout': function() {
      window.dispatchEvent(new Event('resize'));
    }
  },
  async created() {
    // console.log('0.0.1');
    // console.log(this.$encrypt('admin'));
    // console.log(this.$encrypt('888888'));
    // await fetch('http://192.168.111.120/cgi-bin/login.elf?n=admin&p=5b19019644b4958b8bb1536beffaaa4c', {
    //   'credentials': 'include',
    //   'headers': {
    //     'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:91.0) Gecko/20100101 Firefox/91.0',
    //     'Accept': 'application/json, text/plain, */*',
    //     'Accept-Language': 'zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2',
    //     'Content-Type': 'application/json;charset=utf-8',
    //     'Authorization': 'Bearer Authorization:0.8890299662262486',
    //     'Pragma': 'no-cache',
    //     'Cache-Control': 'no-cache'
    //   },
    //   'referrer': 'http://192.168.111.120/',
    //   'method': 'GET',
    //   'mode': 'cors'
    // });
    this.setHtmlTitle();
    this.setLanguage(this.lang);
    enquireScreen(isMobile => this.setDevice(isMobile));
  },
  mounted() {
    this.setWeekModeTheme(this.weekMode);
  },
  computed: {
    ...mapState('setting', ['layout', 'theme', 'weekMode', 'lang'])
  },
  methods: {
    ...mapMutations('setting', ['setDevice']),
    setWeekModeTheme(weekMode) {
      if (weekMode) {
        document.body.classList.add('week-mode');
      } else {
        document.body.classList.remove('week-mode');
      }
    },
    setLanguage(lang) {
      this.$i18n.locale = lang;
      switch (lang) {
        case 'CN':
          this.locale = require('ant-design-vue/es/locale-provider/zh_CN').default;
          break;
        case 'HK':
          this.locale = require('ant-design-vue/es/locale-provider/zh_TW').default;
          break;
        case 'US':
        default:
          this.locale = require('ant-design-vue/es/locale-provider/en_US').default;
          break;
      }
    },
    setHtmlTitle() {
      const route = this.$route;
      const key = route.path === '/' ? 'home.name' : getI18nKey(route.matched[route.matched.length - 1].path);
      document.title = process.env.VUE_APP_NAME + ' | ' + this.$t(key);
    },
    popContainer() {
      return document.getElementById('popContainer');
    }
  }
};
</script>

<style lang="less" scoped>
  #id{
  }
</style>
