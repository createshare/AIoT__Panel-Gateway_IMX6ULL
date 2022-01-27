<template>
  <common-layout>
    <div class="top">
      <div class="header">
        <img alt="logo" class="logo" src="@/assets/img/logo.png" />
        <span class="title">{{ systemName }}</span>
      </div>
    </div>
    <div class="login">
      <a-form :form="form" @submit="onSubmit">
        <a-tabs size="large" :tab-bar-style="{textAlign: 'center'}" style="padding: 0 2px;">
          <a-tab-pane key="1" tab="账户密码登录">
            <a-alert v-if="error" type="error" :closable="true" :message="error" show-icon style="margin-bottom: 24px;" @close="onClose"></a-alert>
            <a-form-item>
              <a-input
                v-decorator="['name', {rules: [{ required: true, message: '请输入账户名', whitespace: true}]}]"
                autocomplete="autocomplete"
                size="large"
                placeholder="请输入账户名"
              >
                <a-icon slot="prefix" type="user"></a-icon>
              </a-input>
            </a-form-item>
            <a-form-item>
              <a-input
                v-decorator="['password', {rules: [{ required: true, message: '请输入密码', whitespace: true}]}]"
                size="large"
                placeholder="请输入密码"
                autocomplete="autocomplete"
                type="password"
              >
                <a-icon slot="prefix" type="lock"></a-icon>
              </a-input>
            </a-form-item>
          </a-tab-pane>
        </a-tabs>
        <div>
          <a-checkbox :checked="true">自动登录</a-checkbox>
          <a style="float: right" @click="resetPasswordVisible = true">重置密码</a>
        </div>
        <a-form-item>
          <a-button :loading="logging" style="width: 100%;margin-top: 24px" size="large" html-type="submit" type="primary">登录</a-button>
        </a-form-item>
      </a-form>
    </div>
    <a-modal
      ref="resetPasswordModal"
      title="重置密码"
      :visible="resetPasswordVisible"
      :confirm-loading="confirmLoading"
      @ok="resetPassword"
      @cancel="resetPasswordVisible = false"
    >
      <a-form ref="resetPassForm" :form="resetPassForm" layout="horizontal" label-align="left" :label-col="{ span: 5 }" :wrapper-col="{ span: 19 }">
        <a-form-item label="用户名称">
          <a-input
            v-decorator="['userName', {rules: [{ required: true, message: '请输入用户名称', whitespace: true},inputLenRule,patternRule]}]"
            placeholder="请输入用户名称"
          ></a-input>
        </a-form-item>
      </a-form>
    </a-modal>
  </common-layout>
</template>

<script>
import CommonLayout from '@/layouts/CommonLayout';
import { login, getRoutesConfig } from '@/services/user';
import { setAuthorization } from '@/utils/request';
import { loadRoutes } from '@/utils/routerUtil';
import { mapMutations } from 'vuex';
import { userService } from '@/services';

export default {
  name: 'Login',
  components: { CommonLayout },
  data() {
    return {
      logging: false,
      error: '',
      form: this.$form.createForm(this),
      resetPasswordVisible: false,
      confirmLoading: false,
      resetPassForm: this.$form.createForm(this),
      inputLenRule: { max: 16, message: '最长可输入16位' },
      patternRule: { pattern: new RegExp(/^\w+$/, 'g'), message: '只能输入数字、字母、下划线' }
    };
  },
  computed: {
    systemName() {
      return this.$store.state.setting.systemName;
    }
  },
  methods: {
    ...mapMutations('account', ['setUser', 'setPermissions', 'setRoles']),
    resetPassword() {
      this.resetPassForm.validateFields((err) => {
        if (!err) {
          const name = this.resetPassForm.getFieldValue('userName');
          this.confirmLoading = true;
          userService.resetPassword(name).then(res => {
            if (res.data === 'success') {
              this.$message.success('重置成功!');
              this.resetPasswordVisible = false;
            } else {
              this.$message.error('重置密码失败!不存在该用户');
            }
          }).catch(e => {
            console.error(e);
          }).finally(() => {
            this.confirmLoading = false;
          });
        }
      });
    },
    onSubmit(e) {
      e.preventDefault();
      this.form.validateFields((err) => {
        if (!err) {
          this.logging = true;
          const name = this.form.getFieldValue('name');
          const password = this.form.getFieldValue('password');
          login(name, this.$encrypt(password)).then(this.afterLogin).catch(e => {
            console.error(e);
          });
        }
      });
    },
    afterLogin(res) {
      this.logging = false;
      const loginRes = res.data;
      const STATUS = {
        NO_USER: 'ename',
        ERR_PASS: 'epass',
        SUCCESS: 'success'
      };
      // 数据未返回,这里仅做模拟
      const permissions = [{ id: 'queryForm', operation: ['add', 'edit'] }];
      const roles = [{ id: 'admin', operation: ['add', 'edit', 'delete'] }];
      const token = 'Authorization:' + Math.random();
      const expireAt = new Date(new Date().getTime() + 24 * 60 * 60 * 1000);
      switch (loginRes) {
        case STATUS.NO_USER:
          this.error = '用户不存在!';
          break;
        case STATUS.ERR_PASS:
          this.error = '密码错误!';
          break;
        case STATUS.SUCCESS:
          this.setUser({ name: res.config.params.n, avatar: 'https://gw.alipayobjects.com/zos/rmsportal/cnrhVkzwxjPwAaCfPbdc.png' });
          this.setPermissions(permissions);
          this.setRoles(roles);
          setAuthorization({ token: token, expireAt: new Date(expireAt) });
          this.$router.push('/home');
          this.$message.success('登录成功!', 3);
          // api获取路由配置,这里暂时没有
          // this.getRoutes();
          break;
        default:
          this.error = 'unknown error!';
          break;
      }
    },
    getRoutes() {
      getRoutesConfig().then(result => {
        const routesConfig = result.data.data;
        loadRoutes(routesConfig);
      });
    },
    onClose() {
      this.error = false;
    }
  }
};
</script>

<style lang="less" scoped>
  .common-layout{
    .top {
      text-align: center;
      .header {
        height: 44px;
        line-height: 44px;
        a {
          text-decoration: none;
        }
        .logo {
          height: 44px;
          vertical-align: top;
          margin-right: 16px;
        }
        .title {
          font-size: 33px;
          color: @title-color;
          font-family: 'Myriad Pro', 'Helvetica Neue', Arial, Helvetica, sans-serif;
          font-weight: 600;
          position: relative;
          top: 2px;
        }
      }
      .desc {
        font-size: 14px;
        color: @text-color-second;
        margin-top: 12px;
        margin-bottom: 40px;
      }
    }
    .login{
      width: 368px;
      margin: 0 auto;
      @media screen and (max-width: 576px) {
        width: 95%;
      }
      @media screen and (max-width: 320px) {
        .captcha-button{
          font-size: 14px;
        }
      }
      .icon {
        font-size: 24px;
        color: @text-color-second;
        margin-left: 16px;
        vertical-align: middle;
        cursor: pointer;
        transition: color 0.3s;

        &:hover {
          color: @primary-color;
        }
      }
    }
  }
</style>
