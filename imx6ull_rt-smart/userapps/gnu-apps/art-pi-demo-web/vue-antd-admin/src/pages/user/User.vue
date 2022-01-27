<template>
  <div class="new-page" :style="`min-height: ${pageMinHeight}px`">
    <advance-table
      :columns="columns"
      :data-source="dataSource"
      :loading="loading"
      row-key="userId"
      :format-conditions="true"
      bordered
      :pagination="{
        current: page,
        pageSize: pageSize,
        total: total,
        showSizeChanger: true,
        showLessItems: true,
        showQuickJumper: true,
        showTotal: (total, range) => `第 ${range[0]}-${range[1]} 条，总计 ${total} 条`,
        onChange: onPageChange,
        onShowSizeChange: onSizeChange,
      }"
      @search="onSearch"
      @refresh="onRefresh"
      @reset="onReset"
    >
      <template slot="title">
        <a-button @click="addUserModalVisible=true">添加用户</a-button>
      </template>
      <template slot="options" slot-scope="{record}">
        <a @click="editUser=record;changePassModalVisible = true">
          <a-icon type="edit"></a-icon>修改密码
        </a>
        <a-divider type="vertical"></a-divider>
        <a-popconfirm
          v-if="dataSource.length"
          title="确定删除该用户?"
          @confirm="() => deleteUser(record)"
        >
          <a href="javascript:;"><a-icon type="delete"></a-icon>删除</a>
        </a-popconfirm>
      </template>
    </advance-table>
    <a-modal
      ref="addUserModal"
      title="增加用户"
      :visible="addUserModalVisible"
      :confirm-loading="confirmLoading"
      @ok="addUser"
      @cancel="addUserModalVisible = false"
    >
      <a-form ref="addUserForm" :form="addUserForm" layout="horizontal" label-align="left" :label-col="{ span: 5 }" :wrapper-col="{ span: 19 }">
        <a-form-item label="用户名称">
          <a-input
            v-decorator="['userName', {rules: [{ required: true, message: '请输入用户名称', whitespace: true},inputLenRule,patternRule]}]"
            placeholder="请输入用户名称"
          ></a-input>
        </a-form-item>
        <a-form-item label="用户密码">
          <a-input
            v-decorator="['password', {rules: [{ required: true, message: '请输入密码', whitespace: true},inputLenRule,patternRule]}]"
            placeholder="请输入密码"
            type="password"
          ></a-input>
        </a-form-item>
        <a-form-item label="确认密码">
          <a-input
            v-decorator="['confirmPassword', {rules: [{ required: true, message: '请确认密码', whitespace: true},inputLenRule,patternRule,{validator:checkConfirmPassword}]}]"
            placeholder="请确认密码"
            type="password"
          ></a-input>
        </a-form-item>
        <a-form-item label="用户角色">
          <a-select
            v-decorator="['role', {initialValue:'admin',rules: [{ required: true, message: '请选择角色', whitespace: true}]}]"
          >
            <a-select-option value="admin">admin</a-select-option>
          </a-select>
        </a-form-item>
      </a-form>
    </a-modal>
    <a-modal
      ref="changPassModal"
      title="修改密码"
      :visible="changePassModalVisible"
      :confirm-loading="confirmLoading"
      @ok="changPassWord"
      @cancel="changePassModalVisible = false"
    >
      <a-form ref="changePassForm" :form="changePassForm" layout="horizontal" label-align="left" :label-col="{ span: 5 }" :wrapper-col="{ span: 19 }">
        <a-form-item label="用户名称">
          <span v-if="editUser">{{ editUser.userName }}</span>
        </a-form-item>
        <a-form-item label="当前密码">
          <a-input
            v-decorator="['oldPassword', {rules: [{ required: true, message: '请输入当前密码', whitespace: true},inputLenRule,patternRule]}]"
            placeholder="请输入当前密码"
            type="password"
          ></a-input>
        </a-form-item>
        <a-form-item label="新密码">
          <a-input
            v-decorator="['password', {rules: [{ required: true, message: '请输入新密码', whitespace: true},inputLenRule,patternRule]}]"
            placeholder="请输入新密码"
            type="password"
          ></a-input>
        </a-form-item>
        <a-form-item label="确认密码">
          <a-input
            v-decorator="['confirmPassword', {rules: [{ required: true, message: '请确认密码', whitespace: true},inputLenRule,patternRule,{validator: checkConfirmPassword}]}]"
            placeholder="请确认密码"
            type="password"
          ></a-input>
        </a-form-item>
      </a-form>
    </a-modal>
  </div>
</template>

<script>
import { mapGetters, mapState } from 'vuex';
import i18n from './i18n';
import AdvanceTable from '@/components/table/advance/AdvanceTable';
import { userService } from '@/services';
export default {
  name: 'User',
  i18n: i18n,
  components: { AdvanceTable },
  data() {
    const checkConfirmPassword = (rule, value, callback) => {
      let password = '';
      if (this.addUserModalVisible) {
        password = this.addUserForm.getFieldValue('password');
      } else if (this.changePassModalVisible) {
        password = this.changePassForm.getFieldValue('password');
      }
      if (password !== value) {
        callback(new Error('两次密码输入不一致'));
      } else {
        callback();
      }
    };

    return {
      columns: [{
        title: '序号',
        dataIndex: 'index',
        searchAble: false,
        align: 'center'
      }, {
        title: '用户名称',
        dataIndex: 'userName',
        searchAble: true,
        align: 'center'
      }, {
        title: '角色',
        dataIndex: 'role',
        searchAble: false,
        align: 'center'
      }, {
        title: '创建时间',
        dataIndex: 'createTime',
        dataType: 'datetime',
        align: 'center'
      }, {
        searchAble: false,
        title: '操作',
        dataIndex: 'options',
        scopedSlots: { customRender: 'options' },
        align: 'center'
      }],
      loading: false,
      page: 1,
      pageSize: 10,
      total: 0,
      dataSource: [],
      conditions: {},
      confirmLoading: false,
      addUserModalVisible: false,
      changePassModalVisible: false,
      addUserForm: this.$form.createForm(this),
      editUser: null,
      changePassForm: this.$form.createForm(this),
      inputLenRule: { max: 16, message: '最长可输入16位' },
      patternRule: { pattern: new RegExp(/^\w+$/, 'g'), message: '只能输入数字、字母、下划线' },
      checkConfirmPassword
    };
  },
  computed: {
    ...mapState('setting', ['pageMinHeight']),
    ...mapGetters('account', ['user'])
  },
  watch: {
    addUserModalVisible() {
      this.addUserForm.resetFields();
    },
    changePassModalVisible() {
      this.changePassForm.resetFields();
    }
  },
  created() {
    this.getTableData();
  },
  methods: {
    getTableData() {
      this.loading = true;
      userService.userList().then(res => {
        console.log(res);
        // 因后台限制需要去除最后一个'&'符号
        const users = res.data.replaceAll('\u0000', ':').substr(0, res.data.length - 1).split('&');
        this.dataSource = users.map((user, index) => {
          const userArr = user.split(';');
          return {
            index: index + 1,
            userName: userArr[0],
            role: userArr[2],
            createTime: userArr[3]
          };
        });
      }).catch(e => {
        console.error(e);
      }).finally(() => {
        this.loading = false;
      });
      // const {page, pageSize, conditions} = this
      // api
      // getDataList({page, pageSize, ...conditions}).then(result => {
      //   const {list, page, pageSize, total} = result.data.data
      //   this.dataSource = list
      //   this.page = page
      //   this.total = total
      //   this.pageSize = pageSize
      //   this.loading = false
      // })
    },
    addUser() {
      this.addUserForm.validateFields((err) => {
        if (!err) {
          const name = this.addUserForm.getFieldValue('userName');
          const password = this.addUserForm.getFieldValue('password');
          this.confirmLoading = true;
          userService.addUser(name, this.$encrypt(password)).then(res => {
            if (res.data === 'success') {
              // 删除成功后需要刷新页面
              this.getTableData();
              this.addUserModalVisible = false;
            } else {
              this.$message.error('添加失败!已存在该用户');
            }
          }).catch(e => {
            console.error(e);
          }).finally(() => {
            this.confirmLoading = false;
          });
        }
      });
    },
    changPassWord() {
      this.changePassForm.validateFields((err) => {
        if (!err) {
          const name = this.editUser.userName;
          const password = this.changePassForm.getFieldValue('password');
          const oldPassword = this.changePassForm.getFieldValue('oldPassword');
          this.confirmLoading = true;
          userService.changePassword(name, this.$encrypt(password), this.$encrypt(oldPassword)).then(res => {
            if (res.data === 'success') {
              // 删除成功后需要刷新页面
              this.getTableData();
              this.changePassModalVisible = false;
            } else {
              this.$message.error('修改失败!密码不正确');
            }
          }).catch(e => {
            console.error(e);
          }).finally(() => {
            this.confirmLoading = false;
          });
        }
      });
    },
    deleteUser(user) {
      if (user.userName === this.user.name) {
        this.$message.error('删除失败! 不能删除本用户');
        return;
      }
      userService.delUser(user.userName).then(res => {
        if (res.data === 'success') {
          // 删除成功后需要刷新页面
          this.getTableData();
        } else {
          this.$message.error('删除失败! 不能删除admin');
        }
      }).catch(e => {
        console.error(e);
      });
    },
    onSearch(conditions, searchOptions) {
      console.log(searchOptions);
      this.page = 1;
      this.conditions = conditions;
      this.getTableData();
    },
    onSizeChange(current, size) {
      this.page = 1;
      this.pageSize = size;
      this.getTableData();
    },
    onRefresh(conditions) {
      this.conditions = conditions;
      this.getTableData();
    },
    onReset(conditions) {
      this.conditions = conditions;
      this.getTableData();
    },
    onPageChange(page, pageSize) {
      this.page = page;
      this.pageSize = pageSize;
      this.getTableData();
    }
  }
};
</script>

<style scoped lang="less">
@import "index";
</style>
