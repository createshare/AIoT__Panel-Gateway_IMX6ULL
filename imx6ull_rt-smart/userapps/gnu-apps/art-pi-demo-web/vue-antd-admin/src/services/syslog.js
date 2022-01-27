import { METHOD, request } from '@/utils/request';
import { SYS_LOG_LIST, SYS_LOG_DEL } from '@/services/api';

/**
 * 系统日志服务
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function getSysLogList() {
  return request(SYS_LOG_LIST, METHOD.GET, {
  });
}

/**
 * 删除日志
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function delSysLog(time) {
  return request(SYS_LOG_DEL, METHOD.GET, {
    t: time
  });
}
export default {
  getSysLogList,
  delSysLog
};
