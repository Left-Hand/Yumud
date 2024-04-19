"use strict";
// / <reference path="./custom.d.ts" />
Object.defineProperty(exports, "__esModule", { value: true });
var fs = require("fs");
function main() {
    try {
        // 尝试读取文件内容
        var content = fs.readFileSync('hello', 'utf-8');
        // 获取调用次数（从文件内容中提取）
        var count = parseInt(content.trim());
        // 如果内容不是数字或文件为空，则设定调用次数为0
        if (isNaN(count)) {
            count = 0;
        }
        // 将调用次数加一
        count++;
        // 将新的调用次数写入文件
        fs.writeFileSync('hello', count.toString());
    }
    catch (error) {
        if (error.code === 'ENOENT') {
            // 如果文件不存在，则创建文件并写入调用次数为1
            fs.writeFileSync('hello', '1');
        }
        else {
            console.error('Error:', error.message);
        }
    }
}
main();
// class Person {
//     name: string;
//     constructor(name: string) {
//       this.name = name;
//     }
//     sayHello() {
//       console.log(`Hello, my name is ${this.name}`);
//     }
//   }
// const person = new Person("Alice");
// person.sayHello();
