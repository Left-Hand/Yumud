    // [[maybe_unused]] auto plot_april = [&](const Vertexs & vertex, const int index, const iq16 dir){
    //     painter.bindImage(sketch);
    //     painter.setColor(RGB565::FUCHSIA);

    //     painter.drawPolygon(vertex.begin(), vertex.size());
    //     auto rect = Rect2i(vertex.begin(), vertex.size());
    //     painter.setColor(RGB565::RED);
    //     painter.drawString(rect.position + Vec2i{4,4}, toString(index));

    //     painter.setColor(RGB565::BLUE);
    //     painter.drawFilledCircle(rect.get_center() + Vec2(12, 0).rotated(dir), 3);
    //     painter.bindImage(tftDisplayer);
    // };