'use strict';

// 内存中保存上一次输入的值
let lastInput = null;

hexo.extend.filter.register('server_middleware', function(app) {
  // ============================================
  // 接口1: GET /api/input/:value
  // 用法示例:
  //   GET /api/input/123      → 保存数字 123
  //   GET /api/input/hello    → 保存字符串 "hello"
  //   GET /api/input/abc123   → 保存字符串 "abc123"
  // ============================================
  app.use('/api/input', (req, res, next) => {
    // req.url 此时是 /123 或 /hello 这样的路径
    const match = req.url.match(/^\/([a-zA-Z0-9]+)$/);
    if (!match) return next();

    lastInput = match[1];

    res.setHeader('Content-Type', 'application/json');
    res.end(JSON.stringify({
      success: true,
      message: '输入已保存',
      value: lastInput
    }));
  });

  // ============================================
  // 接口2: GET /api/last-input
  // 返回上一次通过接口1输入的值
  // ============================================
  app.use('/api/last-input', (req, res, next) => {
    // 只处理完全匹配 /api/last-input 的情况
    if (req.url !== '/' && req.url !== '') return next();

    res.setHeader('Content-Type', 'application/json');
    res.end(JSON.stringify({
      success: true,
      value: lastInput
    }));
  });
});
