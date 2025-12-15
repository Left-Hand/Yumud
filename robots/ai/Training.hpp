// using UnityEngine;

// namespace JufGame.AI.ANN
// {
//     public abstract class Training
//     {
//         public NeuralNet TrainingNet;//需要训练的神经网络
//         public float[][] InputSet => inputSet;//训练输入集
        
//         /*没有「训练输出集」是因为并非所有类型的神经网络都需要「训练输出」
//         所以它不是基类必需的，当然，这些就是题外话了*/
        
//         protected float[][] inputSet;
//         [SerializeField] protected int maxEpochs;//最大迭代次数
        
//         public Training(NeuralNet initedNet, int maxEpochs)
//         {
//             this.maxEpochs = maxEpochs;
//             TrainingNet = initedNet;
//         }

//         public void SetInput(float[][] inputSet)//设置训练输入集
//         {
//             this.inputSet = inputSet;
//         }
//         public abstract bool IsTrainEnd();//是否训练完成
//         public abstract void Train(); //不断训练神经网络
//         public abstract void Train_OneTime();//训练（迭代）一次神经网络

//         //打印神经网络输出的结果，调试用的
//         public static void DebugNetRes(NeuralNet net, float[][] testInput)
//         {
//             for(size_t i = 0; i < testInput.GetLength(0); ++i)
//             {
//                 var res = net.CalcNet(testInput[i]);
//                 for(int j = 0; j < res.Length; ++j)
//                 {
//                     Debug.Log("检验结果 " + i + " = " + res[j]);
//                 }
//             }
//         }
//     }
// }