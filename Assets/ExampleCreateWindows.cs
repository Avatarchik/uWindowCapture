﻿using UnityEngine;
using uWindowCapture;
using System.Collections.Generic;

public class ExampleCreateWindows : MonoBehaviour
{
    [SerializeField] GameObject windowPrefab;
    [SerializeField] float baseWidth = 10000f;
    Dictionary<System.IntPtr, ExampleRenderWindow> windows_ = new Dictionary<System.IntPtr, ExampleRenderWindow>();

    void Start()
    {
        Manager.onWindowAdded += OnWindowAdded;
        Manager.onWindowRemoved += OnWindowRemoved;

        foreach (var pair in Manager.windows) {
            OnWindowAdded(pair.Value);
        }
    }

    void Update()
    {
        UpdateWindows();
    }

    void OnWindowAdded(Window window)
    {
        if (!windowPrefab) return;

        var obj = Instantiate(windowPrefab, transform) as GameObject;
        var renderer = obj.GetComponent<ExampleRenderWindow>();
        if (renderer) {
            renderer.window = window;
            windows_.Add(window.handle, renderer);
        }
    }

    void OnWindowRemoved(Window window)
    {
        if (windows_.ContainsKey(window.handle)) {
            Destroy(windows_[window.handle].gameObject);
            windows_.Remove(window.handle);
        }
    }

    void UpdateWindows()
    {
        var pos = Vector3.zero;
        var preWidth = 0f;

        var enumerator = windows_.GetEnumerator();
        while (enumerator.MoveNext()) {
            var window = enumerator.Current.Value.window;
            var transform = enumerator.Current.Value.transform;

            var title = window.title;
            transform.name = !string.IsNullOrEmpty(title) ? title : "-No Name-";

            var width = window.width / baseWidth;
            var height = window.height / baseWidth;
            transform.localEulerAngles = new Vector3(-90f, 0f, 0f);
            transform.localScale = new Vector3(width, 1f, height);
            pos += new Vector3(10 * (preWidth + width) / 2, 0f, 0f);
            transform.position = pos;
            preWidth = width;
        }
    }
}
