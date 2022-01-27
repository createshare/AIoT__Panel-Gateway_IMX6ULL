import TabsView from '@/layouts/tabs/TabsView';
// import BlankView from '@/layouts/BlankView'
// import PageView from '@/layouts/PageView'

// 路由配置
const options = {
  routes: [
    {
      path: '/login',
      name: '登录页',
      component: () => import('@/pages/login')
    },
    {
      path: '*',
      name: '404',
      component: () => import('@/pages/exception/404')
    },
    {
      path: '/403',
      name: '403',
      component: () => import('@/pages/exception/403')
    },
    {
      path: '/',
      name: 'index',
      component: TabsView,
      redirect: '/login',
      children: [
        {
          path: 'home',
          name: 'home',
          meta: {
            icon: 'home'
          },
          component: () => import('@/pages/home')
        },
        {
          path: 'user',
          name: '用户管理',
          meta: {
            icon: 'user'
          },
          component: () => import('@/pages/user')
        },
        {
          path: 'led',
          name: 'LED Configuration',
          meta: {
            icon: 'bulb'
          },
          component: () => import('@/pages/leds')
        },
        {
          path: 'sysConf',
          name: 'sysConf',
          meta: {
            icon: 'tool'
          },
          component: () => import('@/pages/sysConf')
        },
        {
          path: 'sysLog',
          name: 'sysLog',
          meta: {
            icon: 'snippets'
          },
          component: () => import('@/pages/sysLog')
        }
      ]
    }
  ]
};

export default options;
