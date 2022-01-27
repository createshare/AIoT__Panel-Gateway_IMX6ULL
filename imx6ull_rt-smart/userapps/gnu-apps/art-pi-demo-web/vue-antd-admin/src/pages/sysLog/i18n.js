import { mergeDefaultI18n } from '@/utils/i18n';

const i18n = {
  messages: {
    CN: {
      'System Log': '系统日志',
      'syslog': {
        'Message': '消息',
        'Enter keywords to filter': '输入关键字进行筛选',
        'Datetime': '日期时间',
        'Facility': '类型',
        'Level': '级别',
        'Operate': '操作',
        'Detail': '详情',
        'Index': '序号',
        'facility': {
          'normal': '普通', 'warn': '警告', 'error': '错误'
        },
        'level': {
          'low': '低', 'middle': '中', 'high': '高'
        },
        'operate': {
          'login': '用户登录', 'adduser': '增加用户', 'chpass': '修改密码', 'resetpass': '重置密码', 'deluser': '删除用户'
        }
      }
    },
    HK: {
      'System Log': '系統日誌',
      'syslog': {
        'Message': '消息',
        'Enter keywords to filter': '輸入關鍵字進行篩選',
        'Datetime': '日期時間',
        'Facility': '類型',
        'Level': '級別'
      }
    },
    US: {
      'System Log': 'System Log',
      'syslog': {
        'Message': 'Message',
        'Enter keywords to filter': 'Enter keywords to filter',
        'Datetime': 'Datetime',
        'Facility': 'Facility',
        'Level': 'Level',
        'Operate': 'Operate',
        'Detail': 'Detail',
        'Index': 'Index',
        'facility': {
          'normal': 'normal', 'warn': 'warn', 'error': 'error'
        },
        'level': {
          'low': 'low', 'middle': 'middle', 'high': 'high'
        },
        'operate': {
          'login': 'login', 'adduser': 'add user', 'chpass': 'change password', 'resetpass': 'reset password', 'deluser': 'delete user'
        }
      }
    },
    JP: {
      'System Log': 'システムログ',
      'syslog': {
        'Message': 'ニュース',
        'Enter keywords to filter': 'キーワードを入力してスクリーニングを行う',
        'Datetime': '日時',
        'Facility': 'ログ型',
        'Level': 'レベル'
      }
    }
  }
};
export default mergeDefaultI18n(i18n);
