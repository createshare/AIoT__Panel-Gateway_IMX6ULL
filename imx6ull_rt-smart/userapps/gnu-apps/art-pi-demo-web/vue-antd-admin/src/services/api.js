// 跨域代理前缀
const API_PROXY_PREFIX = '';
const BASE_URL = process.env.NODE_ENV === 'production' ? process.env.VUE_APP_API_BASE_URL : API_PROXY_PREFIX;
// const BASE_URL = process.env.VUE_APP_API_BASE_URL;
module.exports = {
  LOGIN: `${BASE_URL}/cgi-bin/login.elf`,
  ROUTES: `${process.env.VUE_APP_API_BASE_URL}/routes`,
  SYS_INFO: `${BASE_URL}/cgi-bin/sysinfo.elf`,
  SYS_TIME: `${BASE_URL}/cgi-bin/gettime.elf`,
  SYS_LOG_LIST: `${BASE_URL}/cgi-bin/getloglist.elf`,
  SYS_LOG_DEL: `${BASE_URL}/cgi-bin/dellog.elf`,
  USER_LIST: `${BASE_URL}/cgi-bin/getuserlist.elf`,
  ADD_USER: `${BASE_URL}/cgi-bin/adduser.elf`,
  DEL_USER: `${BASE_URL}/cgi-bin/deluser.elf`,
  CH_PASS: `${BASE_URL}/cgi-bin/chpass.elf`,
  RE_PASS: `${BASE_URL}/cgi-bin/resetpass.elf`,
  SYS_CONF: `${BASE_URL}/cgi-bin/getsysconf.elf`
};
