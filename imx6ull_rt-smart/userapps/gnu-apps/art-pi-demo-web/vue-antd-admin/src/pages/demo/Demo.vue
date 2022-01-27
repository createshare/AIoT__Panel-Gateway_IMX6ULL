<template>
  <div class="new-page" :style="`min-height: ${pageMinHeight}px`">
    <h1>{{ $t('content') }}</h1>
  </div>
</template>

<script>
import { mapState } from 'vuex';
import i18n from './i18n';
export default {
  name: 'Demo',
  i18n: i18n,
  data() {
    return {
    };
  },
  computed: {
    ...mapState('setting', ['pageMinHeight']),
    desc() {
      return this.$t('description');
    }
  }
};
</script>

<style scoped lang="less">
@import "index";
</style>
