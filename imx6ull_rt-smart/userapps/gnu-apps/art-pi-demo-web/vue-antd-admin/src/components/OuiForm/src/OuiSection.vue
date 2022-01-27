<script>
export default {
  inject: ['ouiForm'],
  provide() {
    return {
      ouiSection: this
    };
  },
  components: {
    OuiSectionContainerDiv: {
      render(h) {
        return h('div', this.$slots.default);
      }
    }
  },
  props: {
    title: String,
    uciConfig: String,
    card: {
      type: Boolean,
      default: true
    }
  },
  data() {
    return {
      loaded: false
    };
  },
  computed: {
    container() {
      if (this.card && !this.columns) return 'a-card';
      return 'oui-section-container-div';
    },
    config() {
      return this.uciConfig || this.ouiForm.uciConfig;
    },
    form() {
      return this.ouiForm.form;
    }
  },
  created() {
    if (this.uciConfig) {
      this.$uci.load(this.uciConfig).then(() => {
        this.loaded = true;
      });
    } else {
      this.loaded = true;
    }
  },
  methods: {
    get(sid, name) {
      return this.form[`${sid}_${name}`];
    },
    set(sid, name, value) {
      this.form[`${sid}_${name}`] = value;
    }
  }
};
</script>
