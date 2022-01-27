import { LOGIN, ROUTES, USER_LIST, ADD_USER, DEL_USER, CH_PASS, RE_PASS } from '@/services/api';
import { request, METHOD, removeAuthorization } from '@/utils/request';

/**
 * 登录服务
 * @param name 账户名
 * @param password 账户密码(经过md5加密)
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function login(name, password) {
  return request(LOGIN, METHOD.GET, {
    n: name,
    p: password
  });
}

/**
 * 用户列表
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function userList() {
  return request(USER_LIST, METHOD.GET, {
  });
}
/**
 * 增加用户
 * @param name 账户名
 * @param password 账户密码(经过md5加密)
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function addUser(name, password) {
  return request(ADD_USER, METHOD.GET, {
    n: name,
    p: password,
    r: 'admin'
  });
}
/**
 * 删除用户
 * @param name 账户名
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function delUser(name) {
  return request(DEL_USER, METHOD.GET, {
    n: name
  });
}
/**
 * 修改密码
 * @param name 账户名
 * @param password 新密码
 * @param oldPassword 原密码
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function changePassword(name, password, oldPassword) {
  return request(CH_PASS, METHOD.GET, {
    n: name,
    p: oldPassword,
    a: password
  });
}
/**
 * 重置密码
 * @param name 账户名
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function resetPassword(name) {
  return request(RE_PASS, METHOD.GET, {
    n: name
  });
}
export async function getRoutesConfig() {
  return request(ROUTES, METHOD.GET);
}

/**
 * 退出登录
 */
export function logout() {
  localStorage.removeItem(process.env.VUE_APP_ROUTES_KEY);
  localStorage.removeItem(process.env.VUE_APP_PERMISSIONS_KEY);
  localStorage.removeItem(process.env.VUE_APP_ROLES_KEY);
  removeAuthorization();
}
export default {
  login,
  logout,
  getRoutesConfig,
  userList,
  addUser,
  delUser,
  changePassword,
  resetPassword
};
