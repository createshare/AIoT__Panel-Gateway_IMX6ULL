import { METHOD, request } from '@/utils/request';
import { SYS_CONF } from '@/services/api';

/**
 * @enum {{STATUS: string, INFO: string, OFF: string, ON: string}}
 */
export const SysConfType = {
  GET: '0', // 获取信息
  SET_SYS_INFO_INTERVAL: '1', // 保存系统信息刷新间隔
  SET_LED_INFO_INTERVAL: '2', // 保存LED状态刷新间隔
  SET_SYS_TIME_INTERVAL: '3' // 系统时间同步间隔
};

/**
 * 获取系统配置
 * @param {SysConfType} type
 * @param {Number} val
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function sysConf(type, val = 0) {
  return request(SYS_CONF, METHOD.GET, {
    type, val
  });
}
export default {
  SysConfType,
  sysConf
};
