// using System;
// using UnityEngine;

// namespace JufGame.AI.ANN
// {
//     [Serializable]
//     public class Layer
//     {
//         public Neuron[] Neurons => neurons;//存储神经元
//         public float[] Output => output;//存储各神经元激活函数输出
//         [SerializeField] private Neuron[] neurons;
//         [SerializeField] private float[] output;
//         public Layer(int neuronCount)
//         {
//             output = new float[neuronCount];
//             neurons = new Neuron[neuronCount];
//         }
//         //对层中的每个神经元的权重进行初始化
//         public void InitWeights(int weightCount, InitWFunc.Type initType)
//         {
//             for(int i = 0; i < neurons.Length; ++i)
//             {
//                 neurons[i] = new Neuron(weightCount);
//                 InitWFunc.InitWeights(initType, neurons[i]);
//             }
//         }
//         //初始化层中每个神经元的额外参数
//         public void InitCache()
//         {
//             for(int i = 0; i < neurons.Length; ++i)
//             {
//                 neurons[i].InitCache();
//             }
//         }
//         //计算该层，实际上就是计算所有神经元的加权和，并求出激活函数的输出
//         public float[] CalcLayer(float[] inputData, ActivationFunc.Type acFuc)
//         {
//             for(int i = 0; i < neurons.Length; ++i)
//             {
//                 neurons[i].CalcSum(inputData);
//             }
//             ActivationFunc.Calc(acFuc, this);
//             return output;
//         }
//     }
// }