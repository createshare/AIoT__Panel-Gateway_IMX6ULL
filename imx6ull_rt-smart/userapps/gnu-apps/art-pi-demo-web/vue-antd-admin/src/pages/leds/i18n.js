import { mergeDefaultI18n } from '@/utils/i18n';

const i18n = {
  messages: {
    CN: {
      'LED Configuration': 'LED配置',
      'leds': {
        'No LEDs available': '无可用LED',
        'Name': '名称',
        'Led Name': 'Led 名称',
        'Default state': '默认状态',
        'Trigger': '触发器',
        'On-State Delay': '通电时间',
        'Time in milliseconds the LED stays on': 'LED保持通电时间（以毫秒为单位）',
        'Off-State Delay': '关闭时间',
        'Time in milliseconds the LED stays off': 'LED保持关闭时间（以毫秒为单位）',
        'Device': '设备',
        'Trigger Mode': '触发模式',
        'Link On': '活动链接',
        'Transmit': '传送',
        'Receive': '接收'
      }
    },
    HK: {
      'LED Configuration': 'LED配置',
      'leds': {
        'No LEDs available': '無可用LED',
        'Name': '名稱',
        'Led Name': 'Led 名稱',
        'Default state': '默認狀態',
        'Trigger': '觸發器',
        'On-State Delay': '通電時間',
        'Time in milliseconds the LED stays on': 'LED保持通電時間（以毫秒為單位）',
        'Off-State Delay': '關閉時間',
        'Time in milliseconds the LED stays off': 'LED保持關閉時間（以毫秒為單位）',
        'Device': '設備',
        'Trigger Mode': '觸發模式',
        'Link On': '活動鏈接',
        'Transmit': '傳送',
        'Receive': '接收'
      }
    },
    US: {
      'LED Configuration': 'LED Configuration',
      'leds': {
        'No LEDs available': 'No LEDs available',
        'Name': 'Name',
        'Led Name': 'Led Name',
        'Default state': 'Default state',
        'Trigger': 'Trigger',
        'On-State Delay': 'On-State Delay',
        'Time in milliseconds the LED stays on': 'Time in milliseconds the LED stays on',
        'Off-State Delay': 'Off-State Delay',
        'Time in milliseconds the LED stays off': 'Time in milliseconds the LED stays off',
        'Device': 'Device',
        'Trigger Mode': 'Trigger Mode',
        'Link On': 'Link On',
        'Transmit': 'Transmit',
        'Receive': 'Receive'
      }
    },
    JP: {
      'LED Configuration': 'LED仕様',
      'leds': {
        'No LEDs available': 'LED非使用',
        'Name': '名称',
        'Led Name': 'Led 名称',
        'Default state': 'デフォルト状態',
        'Trigger': 'トリガー',
        'On-State Delay': '通電時間',
        'Time in milliseconds the LED stays on': 'LEDは通電時間を保持する(ミリ秒単位)。',
        'Off-State Delay': '关闭时间',
        'Time in milliseconds the LED stays off': 'LLEDはオフ時間(ミリ秒単位)を保持する。',
        'Device': '設備',
        'Trigger Mode': 'トリガーモード',
        'Link On': '活動リンク',
        'Transmit': '送信',
        'Receive': '受信'
      }
    }
  }
};
export default mergeDefaultI18n(i18n);
