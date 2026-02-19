// using System;
// using UnityEngine;

// namespace JufGame.AI.ANN
// {
//     [Serializable]
//     public class NeuralNet
//     {
//         public float TargetError = 0.0001f;//预期误差，当损失函数的结果小于它时，就停止训练
//         public float LearningRate = 0.01f;//学习率
//         public int CurEpochs;//记录当前迭代的次数
//         public ActivationFunc.Type hdnAcFunc;//隐藏层激活函数类型
//         public ActivationFunc.Type outAcFunc;//输出层激活函数类型
//         public Layer[] HdnLayers => hdnLayers;//隐藏层
//         public Layer OutLayer => outLayer;//输出层
//         [SerializeField] private Layer[] hdnLayers;
//         [SerializeField] private Layer outLayer;

//         public NeuralNet(int hdnLayerCount, int[] neuronsOfLayers, int outCount, 
//             ActivationFunc.Type hdnAcFnc, ActivationFunc.Type outAcFnc,
//             float targetError = 0.0001f, float learningRate = 0.01f)
//         {
//             outLayer = new Layer(outCount);
//             hdnLayers = new Layer[hdnLayerCount];
//             for(size_t i = 0, j = 0; i < hdnLayerCount; ++i)
//             {
//                 hdnLayers[i] = new Layer(neuronsOfLayers[j]);
//             }
//             hdnAcFunc = hdnAcFnc;
//             outAcFunc = outAcFnc;
//             TargetError = targetError;
//             LearningRate = learningRate;
//         }
//         //初始化各神经元权重
//         public void InitWeights(int inputDataCount, InitWFunc.Type initType)
//         {
//             int neuronNum = inputDataCount;
//             for(size_t i = 0; i < HdnLayers.Length; ++i)
//             {
//                 hdnLayers[i].InitWeights(neuronNum, initType);
//                 neuronNum = HdnLayers[i].Neurons.Length;
//             }
//             outLayer.InitWeights(neuronNum, initType);
//         }
//         //初始化各神经元额外参数列表
//         public void InitCache()
//         {
//             for(size_t i = 0; i < HdnLayers.Length; ++i)
//             {
//                 hdnLayers[i].InitCache();
//             }
//             outLayer.InitCache();
//         }
//         //计算神经网络
//         public float[] CalcNet(float[] inputData)
//         {
//             var curInput = inputData;
//             for(int j = 0; j < hdnLayers.Length; ++j)
//             {
//                 curInput = hdnLayers[j].CalcLayer(curInput, hdnAcFunc);
//             }
//             return outLayer.CalcLayer(curInput, outAcFunc);
//         }
//     }
// }