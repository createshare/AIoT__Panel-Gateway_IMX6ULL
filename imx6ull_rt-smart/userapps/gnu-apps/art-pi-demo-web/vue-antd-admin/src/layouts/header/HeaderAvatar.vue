<template>
  <a-dropdown>
    <div class="header-avatar" style="cursor: pointer">
      <a-avatar class="avatar" size="small" shape="circle" :src="user.avatar"></a-avatar>
      <span class="name">{{ user.name }}</span>
    </div>
    <a-menu slot="overlay" :class="['avatar-menu']">
      <a-menu-item>
        <a-icon type="user"></a-icon>
        <span>个人中心</span>
      </a-menu-item>
      <a-menu-item>
        <a-icon type="setting"></a-icon>
        <span>设置</span>
      </a-menu-item>
      <a-menu-divider></a-menu-divider>
      <a-menu-item @click="logout">
        <a-icon style="margin-right: 8px;" type="poweroff"></a-icon>
        <span>退出登录</span>
      </a-menu-item>
    </a-menu>
  </a-dropdown>
</template>

<script>
import { mapGetters } from 'vuex';
import { logout } from '@/services/user';

export default {
  name: 'HeaderAvatar',
  computed: {
    ...mapGetters('account', ['user'])
  },
  methods: {
    logout() {
      logout();
      this.$router.push('/login');
    }
  }
};
</script>

<style lang="less">
  .header-avatar{
    display: inline-flex;
    .avatar, .name{
      align-self: center;
    }
    .avatar{
      margin-right: 8px;
    }
    .name{
      font-weight: 500;
    }
  }
  .avatar-menu{
    width: 150px;
  }

</style>
