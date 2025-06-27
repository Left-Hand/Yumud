// #pragma once

// #include "Neuron.hpp"

// namespace ymd::ann {

// namespace NeuronUtils{
//     class ActivationFunc
//     {
//         private delegate float FuncCalc(float x);
//         private static FuncCalc curAcFunc;
//         public enum Type
//         {
//             Identify, Softmax, Tanh, Sigmoid, ReLU, LeakyReLU
//         }  
//         //按层使用激活函数计算
//         public static void Calc(Type funcType, Layer layer)
//         {
//             if(funcType == Type.Softmax)
//             {
//                 Softmax_Calc(layer);
//             }
//             else
//             {
//                 curAcFunc = funcType switch
//                 {
//                     Type.Sigmoid => Sigmoid_Calc,
//                     Type.Tanh => Tanh_Calc,
//                     Type.ReLU => ReLU_Calc,
//                     Type.LeakyReLU => LeakyReLU_Calc,
//                     _ => Identify_Calc,
//                 };
//                 for(int i = 0; i < layer.Neurons.Length; ++i)
//                 {
//                     layer.Output[i] = curAcFunc(layer.Neurons[i].Sum);
//                 }
//             }
//         }
//         //根据传入下标index选取层中神经元，并进行求导
//         public static float Diff(Type funcType, Layer layer, int index)
//         {
//             return funcType switch
//             {
//                 Type.Softmax => Softmax_Diff(layer, index),
//                 Type.Sigmoid => Sigmoid_Diff(layer, index),
//                 Type.Tanh => Tanh_Diff(layer, index),
//                 Type.ReLU => ReLU_Diff(layer, index),
//                 Type.LeakyReLU => LeakyReLU_Diff(layer, index),
//                 _ => Identify_Diff(),
//             };   
//         }
        
//         #region 直接输出
//         private static float Identify_Calc(float x)
//         {
//             return x;
//         }
//         private static float Identify_Diff()
//         {
//             return 1;
//         }
//         #endregion

//         #region Softmax
//         private static void Softmax_Calc(Layer layer)
//         {
//             var neurons = layer.Neurons;
//             var expSum = 0.0f;
//             for(int i = 0; i < neurons.Length; ++i)
//             {
//                 layer.Output[i] = MathF.Exp(neurons[i].Sum);
//                 expSum += layer.Output[i];
//             }
//             for(int i = 0; i < neurons.Length; ++i)
//             {
//                 layer.Output[i] /= expSum;
//             }
//         }
//         private static float Softmax_Diff(Layer outLayer, int index)
//         {
//             return outLayer.Output[index] * (1 - outLayer.Output[index]);
//         }
//         #endregion

//         #region Sigmoid
//         private static float Sigmoid_Calc(float x)
//         {
//             return 1.0f / (1.0f + MathF.Exp(-x));
//         }
//         private static float Sigmoid_Diff(Layer outLayer, int index)
//         {
//             return outLayer.Output[index] * (1 - outLayer.Output[index]);
//         }
//         #endregion

//         #region Tanh
//         private static float Tanh_Calc(float x)
//         {
//             var expVal = MathF.Exp(-x);
//             return (1.0f - expVal) / (1.0f + expVal);
//         }
//         private static float Tanh_Diff(Layer outLayer, int index)
//         {
//             return 1.0f - MathF.Pow(outLayer.Output[index], 2.0f);
//         }
//         #endregion

//         #region ReLU
//         public static float ReLU_Calc(float x)
//         {
//             return x > 0 ? x : 0;
//         }
//         public static float ReLU_Diff(Layer outLayer, int index)
//         {
//             return outLayer.Neurons[index].Sum > 0 ? 1 : 0;
//         }
//         #endregion

//         #region LeakyReLU
//         private static float LeakyReLU_Calc(float x)
//         {
//             return x > 0 ? x : 0.01f * x;
//         }
//         private static float LeakyReLU_Diff(Layer outLayer, int index)
//         {
//             return outLayer.Neurons[index].Sum > 0 ? 1 : 0.01f;
//         }
//         #endregion
//     }
// }

// }