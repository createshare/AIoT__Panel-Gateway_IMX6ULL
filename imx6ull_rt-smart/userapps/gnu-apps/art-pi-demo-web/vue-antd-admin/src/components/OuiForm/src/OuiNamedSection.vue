<template>
  <component :is="container" :title="title">
    <slot v-if="section" :s="section"></slot>
  </component>
</template>

<script>
import OuiSection from './OuiSection.vue';

export default {
  name: 'OuiNamedSection',
  mixins: [OuiSection],
  props: {
    name: {
      type: String,
      required: true
    }
  },
  data() {
    return {
      sections: []
    };
  },
  computed: {
    section() {
      return this.sections.length > 0 ? this.sections[0] : null;
    }
  },
  watch: {
    loaded() {
      this.load();
    }
  },
  methods: {
    load() {
      this.sections = this.$uci.sections(this.config).filter(s => s['.name'] === this.name);
    }
  }
};
</script>
