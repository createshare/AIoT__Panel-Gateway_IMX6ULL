<template>
  <div class="editable-cell">
    <div v-if="editable" class="editable-cell-input-wrapper">
      <a-input-number :default-value="value" :min="200" :precision="0" @change="handleChange" @pressEnter="check"></a-input-number><a-icon
        type="check"
        class="editable-cell-icon-check"
        @click="check"
      ></a-icon>
    </div>
    <div v-else class="editable-cell-text-wrapper">
      {{ value || ' ' }}
      <a-icon type="edit" class="editable-cell-icon" @click="edit"></a-icon>
    </div>
  </div>
</template>

<script>
export default {
  name: 'EditableCell',
  props: {
    text: Number
  },
  data() {
    return {
      value: this.text,
      editable: false
    };
  },
  methods: {
    handleChange(e) {
      this.value = e;
    },
    check() {
      this.editable = false;
      this.$emit('change', this.value);
    },
    edit() {
      this.editable = true;
    }
  }
};
</script>

<style scoped lang="less">
@import "index";
</style>
