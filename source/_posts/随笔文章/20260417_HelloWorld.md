---
title: HelloWorld
date: 2026-04-17 16:47:00
tags:工具
---

#### 这里存储一些常用的博客常用的语法，用于快速拷贝

##### 这个是插入音乐和b站视频的代码样例

- 本地音乐


```html
<div class="my-6 p-4 rounded-xl bg-slate-50 dark:bg-slate-800 border border-gray-200 dark:border-slate-700">
  <audio controls class="w-full outline-none h-10">
    <source src="https://你的网址/音频名字.mp3" type="audio/mpeg">
    抱歉，您的浏览器不支持音频播放。
  </audio>
</div>
```

- 网易云音乐


```html
<div class="my-6 flex justify-center">
  <iframe frameborder="no" border="0" marginwidth="0" marginheight="0" width=330 height=86 
          src="//music.163.com/outchain/player?type=2&id=你的歌曲ID&auto=0&height=66">
  </iframe>
</div>
```

- 哔哩哔哩动画的


```yaml
<div class="w-full aspect-video rounded-xl overflow-hidden shadow-sm border border-gray-200 dark:border-slate-800 my-6">
  <iframe src="//player.bilibili.com/player.html?isOutside=true&aid=116402790927565&bvid=BV1oAQgBQEVr&cid=37501338704&p=1" 
          scrolling="no" border="0" frameborder="no" framespacing="0" allowfullscreen="true" 
          class="w-full h-full">
  </iframe>
</div>
```

##### 这个是YAML Front Matter示例

- **随笔文章**

```yaml
---
title: Hello World
date: 2026-04-17 16:47:00
tags:工具
---
```

- **灵感收集**

```yaml
---
title: 玻璃拟态导航栏
date: 2026-04-17 21:30:00
layout: inspiration
cover: demo-inspiration/image-20260416141421462.png
summary: 一种半透明玻璃拟态的顶部导航设计，适合搭配模糊背景和高对比度标题。
---
```

- **作品集合**

如果是主题页用这个：

```yaml
---
title: 「极客阿明」测试
date: 2026-04-15 20:00:00
layout: project
cover: https://images.unsplash.com/photo-1498050108023-c5249f4df085?q=80&w=1200&auto=format&fit=crop
summary: 从零设计的一款极客风个人博客主题，用于记录独立思考与灵感收集。
project: geek-aming-theme-demo
---
```

如果是内容页面用这个

```yaml
---
title: 第一次编写
date: 2026-04-15 20:00:00
project: geek-aming-theme-demo
---
```

