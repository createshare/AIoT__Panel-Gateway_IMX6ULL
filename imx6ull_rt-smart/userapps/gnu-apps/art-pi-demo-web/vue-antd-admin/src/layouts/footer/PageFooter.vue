<template>
  <div class="footer">
    <div class="links">
      <a v-for="(item, index) in linkList" :key="index" target="_blank" :href="item.link ? item.link : 'javascript: void(0)'">
        <a-icon v-if="item.icon" :type="item.icon"></a-icon>{{ item.name }}
      </a>
    </div>
    <div class="copyright">
      <a v-if="copyrightLink" :href="copyrightLink" target="_blank">
        Copyright<a-icon type="copyright"></a-icon>{{ copyright }}
      </a>
      <span v-else>
        Copyright<a-icon type="copyright"></a-icon>{{ copyright }}
      </span>
    </div>
  </div>
</template>

<script>
export default {
  name: 'PageFooter',
  props: ['copyright', 'linkList', 'copyrightLink']
};
</script>

<style lang="less" scoped>
  .footer{
    padding: 48px 16px 24px;
    /*margin: 48px 0 24px;*/
    text-align: center;
    .copyright{
      color: @text-color-second;
      font-size: 14px;
      i {
          margin: 0 4px;
      }
      a {
        color: @text-color-second;
      }
    }
    .links{
      margin-bottom: 8px;
      a:not(:last-child) {
        margin-right: 40px;
      }
      a{
        color: @text-color-second;
        -webkit-transition: all .3s;
        transition: all .3s;
      }
    }
  }
</style>
