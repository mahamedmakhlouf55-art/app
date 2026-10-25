import 'dart:async';
import 'dart:typed_data';
import 'dart:ui' as ui;
import 'package:flutter/material.dart';
import 'package:flutter/foundation.dart';
import 'package:http/http.dart' as http;

/// Displays the EXACT 240x240 RGB565 pixels drawn by Claude's ESP32 canvas.
/// Local Wi-Fi only. It does not provide worldwide remote access.
class CelyahLiveFace extends StatefulWidget {
  final String baseUrl;
  const CelyahLiveFace({super.key, required this.baseUrl});
  @override State<CelyahLiveFace> createState() => _CelyahLiveFaceState();
}
class _CelyahLiveFaceState extends State<CelyahLiveFace> {
  final http.Client _client = http.Client();
  ui.Image? _frame;
  Timer? _timer;
  bool _busy = false;
  String? _error;
  @override void initState() { super.initState(); _start(); }
  @override void didUpdateWidget(covariant CelyahLiveFace oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.baseUrl != widget.baseUrl) { _timer?.cancel(); _start(); }
  }
  void _start() {
    _fetch();
    _timer = Timer.periodic(const Duration(milliseconds: 250), (_) => _fetch());
  }
  Future<void> _fetch() async {
    if (_busy || !mounted) return;
    _busy = true;
    try {
      final url = Uri.parse('${widget.baseUrl.replaceAll(RegExp(r"/+$"), "")}/frame');
      final response = await _client.get(url).timeout(const Duration(seconds: 3));
      if (response.statusCode != 200 || response.bodyBytes.length != 115200) {
        throw StateError('Frame unavailable: ${response.statusCode}');
      }
      final raw = response.bodyBytes;
      final rgba = Uint8List(240 * 240 * 4);
      for (var i = 0; i < 240 * 240; i++) {
        final pixel = raw[i * 2] | (raw[i * 2 + 1] << 8);
        final r = (pixel >> 11) & 31, g = (pixel >> 5) & 63, b = pixel & 31;
        final j = i * 4;
        rgba[j] = (r << 3) | (r >> 2);
        rgba[j + 1] = (g << 2) | (g >> 4);
        rgba[j + 2] = (b << 3) | (b >> 2);
        rgba[j + 3] = 255;
      }
      final completer = Completer<ui.Image>();
      ui.decodeImageFromPixels(rgba, 240, 240, ui.PixelFormat.rgba8888,
          (image) => completer.complete(image));
      final image = await completer.future;
      if (mounted) {
        final old = _frame;
        setState(() { _frame = image; _error = null; });
        // Dispose only after the previous frame is no longer in use.
        WidgetsBinding.instance.addPostFrameCallback((_) => old?.dispose());
      } else { image.dispose(); }
    } catch (e) {
      if (mounted && _frame == null) setState(() => _error = 'اتصل بشبكة CelYah Wi-Fi لعرض الوجه الحقيقي');
      debugPrint('Celyah frame sync: $e');
    } finally { _busy = false; }
  }
  @override void dispose() { _timer?.cancel(); _client.close(); _frame?.dispose(); super.dispose(); }
  @override Widget build(BuildContext context) => Container(
    color: Colors.black,
    child: _frame == null
      ? Center(child: Text(_error ?? 'جاري الاتصال بوش سيليا...', textAlign: TextAlign.center,
          style: const TextStyle(color: Color(0xFFE17BFF))))
      : RawImage(image: _frame, fit: BoxFit.contain, filterQuality: FilterQuality.none),
  );
}
