import Mock from 'mockjs';

Mock.mock(`${process.env.VUE_APP_API_BASE_URL}/routes`, 'get', () => {
  const result = {};
  result.code = 0;
  result.data = [{
    router: 'root',
    children: ['home',
      {
        router: 'parent1',
        children: [{
          router: 'home',
          name: 'demo1',
          authority: {
            permission: 'home',
            role: 'admin'
          }
        }]
      },
      {
        router: 'parent2',
        children: [{
          router: 'home',
          name: 'demo2'
        }]
      },
      {
        router: 'exception',
        children: ['exp404', 'exp403', 'exp500']
      },
      {
        router: 'home',
        icon: 'file-ppt',
        path: 'auth/home',
        name: '验权页面',
        authority: {
          permission: 'form',
          role: 'manager'
        }
      }
    ]
  }];
  return result;
});
