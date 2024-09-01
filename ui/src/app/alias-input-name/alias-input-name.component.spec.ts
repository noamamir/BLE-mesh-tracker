import { ComponentFixture, TestBed } from '@angular/core/testing';

import { AliasInputNameComponent } from './alias-input-name.component';

describe('AliasInputNameComponent', () => {
  let component: AliasInputNameComponent;
  let fixture: ComponentFixture<AliasInputNameComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ AliasInputNameComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(AliasInputNameComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
