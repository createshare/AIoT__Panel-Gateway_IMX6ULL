import { METHOD, request } from '@/utils/request';
import { SYS_INFO, SYS_TIME } from '@/services/api';

/**
 * @enum {{STATUS: string, INFO: string, OFF: string, ON: string}}
 */
export const SysInfoType = {
  OFF: '0', // LED 关
  ON: '1', // LED 开
  LED_STATUS: '2', // LED状态
  INFO: 'i', // 系统信息
  STATUS: 'm' // 运行状态,包括cpu内存存储等实时信息
};
/**
 * 系统信息服务
 * @param {SysInfoType} type
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function getSysInfo(type) {
  return request(SYS_INFO, METHOD.GET, {
    type
  });
}
/**
 * 系统时间
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function getSysTime() {
  return request(SYS_TIME, METHOD.GET);
}

export default {
  SysInfoType,
  getSysInfo,
  getSysTime
};
