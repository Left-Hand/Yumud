class Signature:
    """
    实现CRC-64-WE哈希算法，及DSDL/数据类型/聚合签名的完整计算逻辑
    对应UAVCAN数据类型签名规范，包含哈希扩展、嵌套签名融合、集合聚合
    """
    MASK64 = 0xFFFFFFFFFFFFFFFF
    POLY = 0x42F0E1EBA9EA3693

    def __init__(self, extend_from=None):
        """
        初始化哈希实例
        :param extend_from: 基于已有签名扩展时，传入64位整数签名值
        """
        if extend_from is not None:
            self._crc = (int(extend_from) & self.MASK64) ^ self.MASK64
        else:
            self._crc = self.MASK64

    def add(self, data_bytes):
        """
        向哈希中添加字节/字符串数据（核心CRC-64-WE计算）
        :param data_bytes: 字节串bytes/字符串str，字符串按ord逐字符处理
        """
        if isinstance(data_bytes, str):
            data_bytes = map(ord, data_bytes)
        for b in data_bytes:
            self._crc ^= (b << 56) & self.MASK64
            for _ in range(8):
                if self._crc & (1 << 63):
                    self._crc = ((self._crc << 1) & self.MASK64) ^ self.POLY
                else:
                    self._crc <<= 1
            self._crc &= self.MASK64  # 确保始终64位

    def get_value(self):
        """获取最终64位签名哈希值"""
        return (self._crc & self.MASK64) ^ self.MASK64

    @staticmethod
    def hash_extend(base_hash: int, extend_with: int) -> int:
        """
        哈希扩展算法：将extend_with融合到base_hash中（核心，用于嵌套/聚合）
        :param base_hash: 基础哈希值（64位整数）
        :param extend_with: 待扩展的哈希值（64位整数）
        :return: 扩展后的新64位哈希值
        """
        # 步骤1：保存原始基础哈希
        saved_hash = base_hash
        # 步骤2：初始化签名实例，基于base_hash扩展
        sig = Signature(extend_from=base_hash)
        # 步骤3：将extend_with按小端序逐字节传入（LSB first）
        sig.add(bytes([(extend_with >> (8 * i)) & 0xFF for i in range(8)]))
        # 步骤4：将保存的原始哈希按小端序逐字节传入
        sig.add(bytes([(saved_hash >> (8 * i)) & 0xFF for i in range(8)]))
        # 步骤5：返回扩展后的值
        return sig.get_value()

    @staticmethod
    def compute_dsdl_signature(normalized_def: str) -> int:
        """
        计算DSDL签名：哈希作用于**规范化后的类型定义文本**
        :param normalized_def: 标准化后的DSDL定义字符串（需提前按规则处理）
        :return: 64位DSDL签名值
        """
        sig = Signature()
        sig.add(normalized_def)
        return sig.get_value()

    @staticmethod
    def compute_type_signature(dsdl_sig: int, nested_type_sigs: list[int]) -> int:
        """
        计算数据类型签名：融合DSDL签名与所有嵌套数据结构的签名
        :param dsdl_sig: 当前类型的DSDL签名（64位整数）
        :param nested_type_sigs: 嵌套数据结构的**数据类型签名**列表（按定义顺序）
        :return: 64位数据类型签名值
        """
        current_hash = dsdl_sig
        # 按定义顺序，依次扩展嵌套结构的签名
        for nested_sig in nested_type_sigs:
            current_hash = Signature.hash_extend(current_hash, nested_sig)
        return current_hash

    @staticmethod
    def compute_aggregate_signature(type_name_sig: dict[str, int]) -> int:
        """
        计算聚合签名：针对一组数据类型的集合兼容性校验
        :param type_name_sig: 字典{完整类型名: 数据类型签名}
        :return: 64位聚合签名值
        """
        # 步骤1：按完整名称ASCII字典序**降序**排序
        sorted_items = sorted(type_name_sig.items(), key=lambda x: x[0], reverse=True)
        if not sorted_items:
            raise ValueError("聚合签名计算的类型集合不能为空")
        # 步骤2：初始化哈希为第一个类型的签名
        current_hash = sorted_items[0][1]
        # 步骤3：依次扩展剩余类型的签名
        for _, sig in sorted_items[1:]:
            current_hash = Signature.hash_extend(current_hash, sig)
        return current_hash


# ------------------- 快速使用示例 -------------------
if __name__ == "__main__":
    # 示例1：计算DSDL签名（需传入**规范化后的**定义文本）
    normalized_def = "root.A\n@union\nsaturated float16 foo\ntruncated uint8 bar"
    dsdl_sig = Signature.compute_dsdl_signature(normalized_def)
    print(f"DSDL签名: 0x{dsdl_sig:016X} ({dsdl_sig})")

    # 示例2：计算数据类型签名（无嵌套，则等于DSDL签名）
    type_sig_no_nested = Signature.compute_type_signature(dsdl_sig, [])
    print(f"无嵌套数据类型签名: 0x{type_sig_no_nested:016X} (与DSDL签名一致: {type_sig_no_nested == dsdl_sig})")

    # 示例3：计算数据类型签名（含1个嵌套结构）
    nested_sig = 0x1234567890ABCDEF  # 嵌套结构的**数据类型签名**
    type_sig_with_nested = Signature.compute_type_signature(dsdl_sig, [nested_sig])
    print(f"含嵌套数据类型签名: 0x{type_sig_with_nested:016X}")

    # 示例4：计算聚合签名（3个类型的集合）
    type_set = {
        "root.A": type_sig_with_nested,
        "root.ns1.B": 0x9876543210FEDCBA,
        "root.C": 0xABCDEF1234567890
    }
    aggregate_sig = Signature.compute_aggregate_signature(type_set)
    print(f"聚合签名: 0x{aggregate_sig:016X}")